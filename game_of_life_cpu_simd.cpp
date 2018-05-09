#include <cstdlib>
#include <cstring>
#include <functional>
#include <x86intrin.h>
#include "simulators.hpp"
#include "util.hpp"

/** ==========================================================================
 * Implementation which uses vectors of 16 characters. Uses SSE2 and SSSE3 
 * extensions.
 * ========================================================================== */
static inline __m128i alive_vec16(__m128i count, __m128i state)
{
    return _mm_and_si128(
        _mm_or_si128(
            _mm_cmpeq_epi8(count, _mm_set1_epi8(3)), 
            _mm_and_si128(state, _mm_cmpeq_epi8(count, _mm_set1_epi8(2)))
        ), 
        _mm_set1_epi8(1)
    );
}

static void game_of_life_cpu_vec16(char* board, int width, int height, int gens)
{
    throw_false<std::invalid_argument>(width >= 16, "width of the board must be"
        " at least 16");

    int size = width * height;
    char* buf = new char[size];

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

                cells = alive_vec16(cells, r_cells);
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
            
                /* First vector of every row is a special case because the west
                neighbors wrap around. */
                __m128i cells;
                __m128i n_cells = _mm_loadu_si128((__m128i*)(board + inorth));
                cells = n_cells;
                __m128i ne_cells = _mm_loadu_si128((__m128i*)(board+inorth+1));
                cells = _mm_add_epi8(cells, ne_cells);

                /* The elements are shifted one to the east and the first index
                is replaced by the last cell in the row in order to align the
                west neighbors with the cell vector. A left shift is done 
                because the system is little endian. */
                __m128i nw_cells = _mm_slli_si128(n_cells, 1);
                ((char*)&nw_cells)[0] = board[inorth + width - 1];
                cells = _mm_add_epi8(cells, nw_cells);

                __m128i r_cells = _mm_loadu_si128((__m128i*)(board + irow));
                __m128i e_cells = _mm_loadu_si128((__m128i*)(board + irow + 1));
                cells = _mm_add_epi8(cells, e_cells);
                __m128i w_cells = _mm_slli_si128(r_cells, 1);
                ((char*)&w_cells)[0] = board[irow + width - 1];
                cells = _mm_add_epi8(cells, w_cells);

                __m128i s_cells = _mm_loadu_si128((__m128i*)(board + isouth));
                cells = _mm_add_epi8(cells, s_cells);
                __m128i se_cells = _mm_loadu_si128((__m128i*)(board+isouth+1));
                cells = _mm_add_epi8(cells, se_cells);
                __m128i sw_cells = _mm_slli_si128(s_cells, 1);
                ((char*)&sw_cells)[0] = board[isouth + width - 1];
                cells = _mm_add_epi8(cells, sw_cells);

                cells = alive_vec16(cells, r_cells);
                _mm_storeu_si128((__m128i*)(buf + irow), cells);

                /* Middle vectors simple perform unaligned memory accesses to
                get their east and west neighbors */
                for (int x = 16; x < width - 16; x += 16) {
                    /* Initialize cells to the north neighbors to clear the
                    cells from the previous iteration. */
                    cells = _mm_loadu_si128((__m128i*)(board + inorth + x));
                    nw_cells = _mm_loadu_si128((__m128i*)(board+inorth+x-1));
                    cells = _mm_add_epi8(cells, nw_cells);
                    ne_cells = _mm_loadu_si128((__m128i*)(board+inorth+x+1));
                    cells = _mm_add_epi8(cells, ne_cells);
                    
                    w_cells = _mm_loadu_si128((__m128i*)(board + irow + x - 1));
                    cells = _mm_add_epi8(cells, w_cells);
                    e_cells = _mm_loadu_si128((__m128i*)(board + irow + x + 1));
                    cells = _mm_add_epi8(cells, e_cells);

                    s_cells = _mm_loadu_si128((__m128i*)(board + isouth + x));
                    cells = _mm_add_epi8(cells, s_cells);
                    sw_cells = _mm_loadu_si128((__m128i*)(board+isouth+x-1));
                    cells = _mm_add_epi8(cells, sw_cells);
                    se_cells = _mm_loadu_si128((__m128i*)(board+isouth+x+1));
                    cells = _mm_add_epi8(cells, se_cells);

                    r_cells = _mm_loadu_si128((__m128i*)(board + irow + x));
                    cells = alive_vec16(cells, r_cells);
                    _mm_storeu_si128((__m128i*)(buf + irow + x), cells);
                }

                /* Last vector of every row is a special case because the east
                neighbors wrap around */
                n_cells = _mm_loadu_si128((__m128i*)(board+inorth+width-16));
                cells = n_cells;
                nw_cells = _mm_loadu_si128((__m128i*)(board+inorth+width-17));
                cells = _mm_add_epi8(cells, nw_cells);

                /* The elements are shifted one to the west and the last index
                is replaced by the first cell in the row in order to align the
                east neighbors with the cell vector. A right shift is done 
                because the system is little endian. */
                ne_cells = _mm_srli_si128(n_cells, 1);
                ((char*)&ne_cells)[15] = board[inorth];
                cells = _mm_add_epi8(cells, ne_cells);
                
                r_cells = _mm_loadu_si128((__m128i*)(board+irow+width-16));
                w_cells = _mm_loadu_si128((__m128i*)(board+irow+width-17));
                cells = _mm_add_epi8(cells, w_cells);
                e_cells = _mm_srli_si128(r_cells, 1);
                ((char*)&e_cells)[15] = board[irow];
                cells = _mm_add_epi8(cells, e_cells);

                s_cells = _mm_loadu_si128((__m128i*)(board+isouth+width-16));
                cells = _mm_add_epi8(cells, s_cells);
                sw_cells = _mm_loadu_si128((__m128i*)(board+isouth+width-17));
                cells = _mm_add_epi8(cells, sw_cells);
                se_cells = _mm_srli_si128(s_cells, 1);
                ((char*)&se_cells)[15] = board[isouth];
                cells = _mm_add_epi8(cells, se_cells);

                cells = alive_vec16(cells, r_cells);
                _mm_storeu_si128((__m128i*)(buf + irow + width - 16), cells);
            }
            swap_ptr(char*, board, buf);
        }
    }

    /**
     * Always want both board and buf to be pointing at their original addresses
     * after the simulation. If the number of generations is an odd number, the
     * result would be in buf and buf and board would be flipped.
     */
    if (gens & 1) { 
        swap_ptr(char*, buf, board);
        memcpy(board, buf, size);
    }
    delete[] buf;
}

/**
 * Single thread implementation which uses MMX, SSE, and SSE2 extensions.
 */ 
void game_of_life_cpu_simd(char* board, int width, int height, int gens)
{
    game_of_life_cpu_vec16(board, width, height, gens);
}