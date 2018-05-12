#include <cstdlib>
#include <cstring>
#include <x86intrin.h>
#include "simulators.hpp"
#include "util.hpp"

/**
 * Implementation which uses vectors of 16 8-bit integers. Uses SSE2 and SSSE3 
 * extensions.
 */
static inline __m128i alive_simd_16(__m128i count, __m128i state)
{
    return _mm_and_si128(
        _mm_or_si128(
            _mm_cmpeq_epi8(count, _mm_set1_epi8(3)), 
            _mm_and_si128(state, _mm_cmpeq_epi8(count, _mm_set1_epi8(2)))
        ), 
        _mm_set1_epi8(1)
    );
}

static void game_of_life_cpu_simd_16(char* board, int width, int height, 
    int gens)
{
    throw_false<std::invalid_argument>(width >= 16, "width of the board must be"
        " at least 16");

    int size = width * height;
    char* buf = new char[size];

    /* Special case if the width is equal to the length of the vector type (16) 
    because the row is simply rotated to get the east and west neighbors */
    if (width == 16) {
        for (int i = 0; i < gens; ++i) {
            for (int y = 0; y < height; ++y) {
                int irow = y * width;
                int inorth = (y ? y - 1 : height - 1) * width;
                int isouth = ((y == (height - 1)) ? 0 : (y + 1)) * width;

                __m128i cells;
                __m128i n_cells = _mm_load_si128((__m128i*)(board + inorth));
                cells = n_cells;
                __m128i ne_cells = _mm_alignr_epi8(n_cells, n_cells, 1);
                cells = _mm_add_epi8(cells, ne_cells);
                __m128i nw_cells = _mm_alignr_epi8(n_cells, n_cells, 15);
                cells = _mm_add_epi8(cells, nw_cells);

                __m128i r_cells = _mm_load_si128((__m128i*)(board + irow));
                __m128i e_cells = _mm_alignr_epi8(r_cells, r_cells, 1);
                cells = _mm_add_epi8(cells, e_cells);
                __m128i w_cells = _mm_alignr_epi8(r_cells, r_cells, 15);
                cells = _mm_add_epi8(cells, w_cells);

                __m128i s_cells = _mm_load_si128((__m128i*)(board + isouth));
                cells = _mm_add_epi8(cells, s_cells);
                __m128i se_cells = _mm_alignr_epi8(s_cells, s_cells, 1);
                cells = _mm_add_epi8(cells, se_cells);
                __m128i sw_cells = _mm_alignr_epi8(s_cells, s_cells, 15);
                cells = _mm_add_epi8(cells, sw_cells);

                cells = alive_simd_16(cells, r_cells);
                _mm_store_si128((__m128i*)(buf + irow), cells);
            }
            swap_ptr(char*, board, buf);
        }
    }
    else {
        for (int i = 0; i < gens; ++i) {
            for (int y = 0; y < height; ++y) {
                int irow = y * width;
                int inorth = (y ? y - 1 : height - 1) * width;
                int isouth = ((y == (height - 1)) ? 0 : (y + 1)) * width;

                // Pointers to the start of the north, current, and south rows
                char* rnorth = board + inorth;
                char* row = board + irow;
                char* rsouth = board + isouth;
            
                /* First vector of every row is a special case because the west
                neighbors wrap around. For the west neighbors (nw, w, sw), the
                elements of the north neighbors, the cell themselves, and south 
                neighbors, repectively, are shifted one to the east and the 
                first index is replaced by the last cell in the row. A left 
                shift is done because the system is little endian. */
                __m128i cells;
                __m128i n_cells = _mm_loadu_si128((__m128i*)(rnorth));
                cells = n_cells;
                __m128i ne_cells = _mm_loadu_si128((__m128i*)(rnorth + 1));
                cells = _mm_add_epi8(cells, ne_cells);
                __m128i nw_cells = _mm_slli_si128(n_cells, 1);
                ((char*)&nw_cells)[0] = *(rnorth + width - 1);
                cells = _mm_add_epi8(cells, nw_cells);

                __m128i r_cells = _mm_loadu_si128((__m128i*)(row));
                __m128i e_cells = _mm_loadu_si128((__m128i*)(row + 1));
                cells = _mm_add_epi8(cells, e_cells);
                __m128i w_cells = _mm_slli_si128(r_cells, 1);
                ((char*)&w_cells)[0] = *(row + width - 1);
                cells = _mm_add_epi8(cells, w_cells);

                __m128i s_cells = _mm_loadu_si128((__m128i*)(rsouth));
                cells = _mm_add_epi8(cells, s_cells);
                __m128i se_cells = _mm_loadu_si128((__m128i*)(rsouth + 1));
                cells = _mm_add_epi8(cells, se_cells);
                __m128i sw_cells = _mm_slli_si128(s_cells, 1);
                ((char*)&sw_cells)[0] = *(rsouth + width - 1);
                cells = _mm_add_epi8(cells, sw_cells);

                cells = alive_simd_16(cells, r_cells);
                _mm_storeu_si128((__m128i*)(buf + irow), cells);

                /* Middle vectors simply perform unaligned memory accesses to
                get their east and west neighbors. */
                for (int x = 16; x < width - 16; x += 16) {
                    int ieast = x + 1;
                    int iwest = x - 1;

                    /* Initialize cells to the north neighbors to clear the
                    cells from the previous iteration. */
                    cells = _mm_loadu_si128((__m128i*)(rnorth + x));
                    nw_cells = _mm_loadu_si128((__m128i*)(rnorth + iwest));
                    cells = _mm_add_epi8(cells, nw_cells);
                    ne_cells = _mm_loadu_si128((__m128i*)(rnorth + ieast));
                    cells = _mm_add_epi8(cells, ne_cells);
                    
                    w_cells = _mm_loadu_si128((__m128i*)(row + iwest));
                    cells = _mm_add_epi8(cells, w_cells);
                    e_cells = _mm_loadu_si128((__m128i*)(row + ieast));
                    cells = _mm_add_epi8(cells, e_cells);

                    s_cells = _mm_loadu_si128((__m128i*)(rsouth + x));
                    cells = _mm_add_epi8(cells, s_cells);
                    sw_cells = _mm_loadu_si128((__m128i*)(rsouth + iwest));
                    cells = _mm_add_epi8(cells, sw_cells);
                    se_cells = _mm_loadu_si128((__m128i*)(rsouth + ieast));
                    cells = _mm_add_epi8(cells, se_cells);

                    r_cells = _mm_loadu_si128((__m128i*)(row + x));
                    cells = alive_simd_16(cells, r_cells);
                    _mm_storeu_si128((__m128i*)(buf + irow + x), cells);
                }

                /* Last vector of every row is a special case because the east
                neighbors wrap around. For the east neighbors (ne, e, se), the
                elements of the north neighbors, the cell themselves, and south 
                neighbors, repectively, are shifted one to the west and the 
                last index is replaced by the first cell in the row. A right 
                shift is done because the system is little endian. */
                n_cells = _mm_loadu_si128((__m128i*)(rnorth + width - 16));
                cells = n_cells;
                nw_cells = _mm_loadu_si128((__m128i*)(rnorth + width - 17));
                cells = _mm_add_epi8(cells, nw_cells);
                ne_cells = _mm_srli_si128(n_cells, 1);
                ((char*)&ne_cells)[15] = *rnorth;
                cells = _mm_add_epi8(cells, ne_cells);
                
                r_cells = _mm_loadu_si128((__m128i*)(row + width - 16));
                w_cells = _mm_loadu_si128((__m128i*)(row + width - 17));
                cells = _mm_add_epi8(cells, w_cells);
                e_cells = _mm_srli_si128(r_cells, 1);
                ((char*)&e_cells)[15] = *row;
                cells = _mm_add_epi8(cells, e_cells);

                s_cells = _mm_loadu_si128((__m128i*)(rsouth + width - 16));
                cells = _mm_add_epi8(cells, s_cells);
                sw_cells = _mm_loadu_si128((__m128i*)(rsouth + width - 17));
                cells = _mm_add_epi8(cells, sw_cells);
                se_cells = _mm_srli_si128(s_cells, 1);
                ((char*)&se_cells)[15] = *rsouth;
                cells = _mm_add_epi8(cells, se_cells);

                cells = alive_simd_16(cells, r_cells);
                _mm_storeu_si128((__m128i*)(buf + irow + width - 16), cells);
            }
            swap_ptr(char*, board, buf);
        }
    }

    if (gens & 1) { 
        swap_ptr(char*, buf, board);
        memcpy(board, buf, size);
    }
    delete[] buf;
}

/**
 * Implementation which uses an integer type to simulate SIMD operations. This
 * hack works as follows:
 * 1. An address is interpreted as a pointer to an integer type and is 
 *   dereferenced as such in order to load multiple cells from memory at once.
 * 2. Regular addition of integers is done with the neighbor integer "vectors" 
 *   for every cell integer "vector" to get the neighbor count "vector". The 
 *   case where the sum of one cell will contaminate the next cell because the 
 *   carry in/out at the byte boundaries will never exist since the maximum sum 
 *   for every byte is 8, which is well below 255. The result should be an 
 *   n-byte integer "vector" in which each byte is between 0 and 8.
 * 3. alive_simd_int() performs a bunch of bitwise operations to determine
 *   whether the cells in the integer "vector" are alive or dead in the next 
 *   generation. Do not try to understand it, it just works.
 * 
 * This implementation is required because x86 does not provide intrinsics for 
 * vector sizes smaller than 64-bit. Due to the nature of this implementation,
 * it is not portable and will only work on little endian systems which allow
 * unaligned memory accesses, e.g., x86_64. 
 */
template <class T>
static inline T alive_simd_int(T count, T state)
{
    return (state | count) & (count >> 1) & ~(count >> 2) & ~(count >> 3) &
        (T)0x0101010101010101;
}

template <class T>
static void game_of_life_cpu_simd_int(char* board, int width, int height, 
    int gens)
{
    int vec_len = sizeof(T);
    throw_false<std::invalid_argument>(width >= vec_len, "width of the board "
        "must be at least " + std::to_string(vec_len));
    
    int size = width * height;
    char* buf = new char[size];

    /* Special case if the width is equal to the length of the integer type 
    because the row is simply rotated to get the east and west neighbors */
    if (width == vec_len) {
        for (int i = 0; i < gens; ++i) {
            for (int y = 0; y < height; ++y) {
                int irow = y * width;
                int inorth = (y ? y - 1 : height - 1) * width;
                int isouth = ((y == (height - 1)) ? 0 : (y + 1)) * width;

                T n_cells = *(T*)(board + inorth);
                T nw_cells = (n_cells << 8) | (n_cells >> ((vec_len - 1) * 8));
                T ne_cells = (n_cells >> 8) | (n_cells << ((vec_len - 1) * 8));
                T r_cells = *(T*)(board + irow);
                T w_cells = (r_cells << 8) | (r_cells >> ((vec_len - 1) * 8));
                T e_cells = (r_cells >> 8) | (r_cells << ((vec_len - 1) * 8));
                T s_cells = *(T*)(board + isouth);
                T sw_cells = (s_cells << 8) | (s_cells >> ((vec_len - 1) * 8));
                T se_cells = (s_cells >> 8) | (s_cells << ((vec_len - 1) * 8));
                T cells = n_cells + nw_cells + ne_cells + w_cells + e_cells +
                          s_cells + sw_cells + se_cells;
                cells = alive_simd_int<T>(cells, r_cells);
                *(T*)(buf + irow) = cells;
            }
            swap_ptr(char*, board, buf);
        }
    }
    else {
        for (int i = 0; i < gens; ++i) {
            for (int y = 0; y < height; ++y) {
                int irow = y * width;
                int inorth = (y ? y - 1 : height - 1) * width;
                int isouth = ((y == (height - 1)) ? 0 : (y + 1)) * width;

                // Pointers to the start of the north, current, and south rows
                char* rnorth = board + inorth;
                char* row = board + irow;
                char* rsouth = board + isouth;
            
                /* First vector of every row is a special case because the west
                neighbors wrap around. */
                T n_cells = *(T*)rnorth;
                T nw_cells = n_cells << 8 | *(rnorth + width - 1);
                T ne_cells = *(T*)(rnorth + 1);
                T r_cells = *(T*)row;
                T w_cells = r_cells << 8 | *(row + width - 1);
                T e_cells = *(T*)(row + 1);
                T s_cells = *(T*)rsouth;
                T sw_cells = s_cells << 8 | *(rsouth + width - 1);
                T se_cells = *(T*)(rsouth + 1);
                T cells = n_cells + nw_cells + ne_cells + w_cells + e_cells +
                          s_cells + sw_cells + se_cells;
                cells = alive_simd_int<T>(cells, r_cells);
                *(T*)(buf + irow) = cells;

                /* Middle vectors simply perform unaligned memory accesses to
                get their east and west neighbors. */
                for (int x = vec_len; x < width - vec_len; x += vec_len) {
                    int ieast = x + 1;
                    int iwest = x - 1;

                    n_cells = *(T*)(rnorth + x);
                    nw_cells = *(T*)(rnorth + iwest);
                    ne_cells = *(T*)(rnorth + ieast);
                    w_cells = *(T*)(row + iwest);
                    e_cells = *(T*)(row + ieast);
                    s_cells = *(T*)(rsouth + x);
                    sw_cells = *(T*)(rsouth + iwest);
                    se_cells = *(T*)(rsouth + ieast);
                    cells = n_cells + nw_cells + ne_cells + w_cells + e_cells +
                            s_cells + sw_cells + se_cells;
                    cells = alive_simd_int<T>(cells, *(T*)(row + x));
                    *(T*)(buf + irow + x) = cells;
                }

                /* Last vector of every row is a special case because the east
                neighbors wrap around. */
                n_cells = *(T*)(rnorth + width - vec_len);
                nw_cells = *(T*)(rnorth + width - vec_len - 1);
                ne_cells = n_cells >> 8 | ((T)(*rnorth) << ((vec_len - 1) * 8));
                r_cells = *(T*)(row + width - vec_len);
                w_cells = *(T*)(row + width - vec_len - 1);
                e_cells = r_cells >> 8 | ((T)(*row) << ((vec_len - 1) * 8));
                s_cells = *(T*)(rsouth + width - vec_len);
                sw_cells = *(T*)(rsouth + width - vec_len - 1);
                se_cells = s_cells >> 8 | ((T)(*rsouth) << ((vec_len - 1) * 8));
                cells = n_cells + nw_cells + ne_cells + w_cells + e_cells +
                        s_cells + sw_cells + se_cells;
                cells = alive_simd_int<T>(cells, r_cells);
                *(T*)(buf + irow + width - vec_len) = cells;
            }
            swap_ptr(char*, board, buf);
        }
    }

    if (gens & 1) { 
        swap_ptr(char*, buf, board);
        memcpy(board, buf, size);
    }
    delete[] buf;
}

/**
 * Single thread implementation which processes multiple cells as a vector in
 * every iteration.
 */ 
void game_of_life_cpu_simd(char* board, int width, int height, int gens)
{
    if (width >= 16) 
        game_of_life_cpu_simd_16(board, width, height, gens);
    else if (width >= 8)
        game_of_life_cpu_simd_int<uint64_t>(board, width, height, gens);
    else if (width >= 4)
        game_of_life_cpu_simd_int<uint32_t>(board, width, height, gens);
    else if (width >= 2)
        game_of_life_cpu_simd_int<uint16_t>(board, width, height, gens);
    else
        game_of_life_cpu_simd_int<uint8_t>(board, width, height, gens);
}