/**
 * sim_cpu_simd.hpp
 * 
 * Optimized data parallel implementation of Conway's Game of Life using SIMD 
 * operations. Uses SSE2 and SSSE3 extensions.
 * 
 * Due to some of the operations performed in this implementation such as 
 * vector extensions and unaligned memory accesses, this is not portable and is
 * only guaranteed to work on an x86_64 system with the extensions specified 
 * above.
 * 
 * Formerly called game_of_life_cpu_simd.hpp.
 * 
 * Author: Carl Marquez
 * Created on: May 19, 2018
 */
#ifndef __SIM_CPU_SIMD_HPP__
#define __SIM_CPU_SIMD_HPP__

#include <cstring>
#include <x86intrin.h>
#include "sim.hpp"
#include "util.hpp"

/*******************************************************************************
 * CPU SIMD 128-bit vector SSE2/SSSE3
 * 
 * An optimized single-thread implementation which uses SIMD operations to
 * process cells in parallel. 
 ******************************************************************************/

void sim_cpu_simd_16(char* grid, int width, int height, int gens);

/* Calculates the next states of 16 cells in a vector. */
static inline __m128i cpu_simd_16_alive(__m128i count, __m128i state)
{
    return _mm_and_si128(
        _mm_or_si128(
            _mm_cmpeq_epi8(count, _mm_set1_epi8(3)), 
            _mm_and_si128(state, _mm_cmpeq_epi8(count, _mm_set1_epi8(2)))
        ), 
        _mm_set1_epi8(1)
    );
}

/* If the width is equal to 16, simply rotate every row one cell to the east or 
 * west to the get the west and east neighbors, respectively.
 * 
 * The _e in the function name stands for equal (width == size of vector). */
static inline void cpu_simd_16_row_e(char* grid, char* buf, int width, 
    int height, int y, int ynorth, int ysouth)
{
    int irow = y * width;
    int inorth = ynorth * width;
    int isouth = ysouth * width;

    __m128i cells;
    __m128i n_cells = _mm_load_si128((__m128i*)(grid + inorth));
    cells = n_cells;
    __m128i ne_cells = _mm_alignr_epi8(n_cells, n_cells, 1);
    cells = _mm_add_epi8(cells, ne_cells);
    __m128i nw_cells = _mm_alignr_epi8(n_cells, n_cells, 15);
    cells = _mm_add_epi8(cells, nw_cells);

    __m128i r_cells = _mm_load_si128((__m128i*)(grid + irow));
    __m128i e_cells = _mm_alignr_epi8(r_cells, r_cells, 1);
    cells = _mm_add_epi8(cells, e_cells);
    __m128i w_cells = _mm_alignr_epi8(r_cells, r_cells, 15);
    cells = _mm_add_epi8(cells, w_cells);

    __m128i s_cells = _mm_load_si128((__m128i*)(grid + isouth));
    cells = _mm_add_epi8(cells, s_cells);
    __m128i se_cells = _mm_alignr_epi8(s_cells, s_cells, 1);
    cells = _mm_add_epi8(cells, se_cells);
    __m128i sw_cells = _mm_alignr_epi8(s_cells, s_cells, 15);
    cells = _mm_add_epi8(cells, sw_cells);

    cells = cpu_simd_16_alive(cells, r_cells);
    _mm_store_si128((__m128i*)(buf + irow), cells);
}

/* If the width of the grid is greater than 16, the first and last vectors of 
 * each row must be handled separately since their west and east neighbors, 
 * respectively, wrap around. The middle vectors simply load the addresses 
 * offset by one in both directions to access the west and east neighbors.
 * 
 * The _g in the function name stands for greater (width > size of vector). */
static inline void cpu_simd_16_row_g(char* grid, char* buf, int width, 
    int height, int y, int ynorth, int ysouth)
{
    int irow = y * width;
    int inorth = ynorth * width;
    int isouth = ysouth * width;

    // Pointers to the start of the north, current, and south rows
    char* rnorth = grid + inorth;
    char* row = grid + irow;
    char* rsouth = grid + isouth;

    /* First vector of every row is a special case because the west neighbors 
    wrap around. To access the west neighbors (nw, w, sw), the vectors of the 
    north neighbors, the cell themselves, and south neighbors, repectively, are 
    shifted one to the east and the first vector element is replaced by the last
    cell in their respective rows. A left shift is done because the system is 
    little endian. */
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

    cells = cpu_simd_16_alive(cells, r_cells);
    _mm_storeu_si128((__m128i*)(buf + irow), cells);

    // Middle vectors
    for (int x = 16; x < width - 16; x += 16) {
        int xeast = x + 1;
        int xwest = x - 1;

        /* Initialize cells to the north neighbors to clear the cells from the 
        previous iteration. */
        cells = _mm_loadu_si128((__m128i*)(rnorth + x));
        nw_cells = _mm_loadu_si128((__m128i*)(rnorth + xwest));
        cells = _mm_add_epi8(cells, nw_cells);
        ne_cells = _mm_loadu_si128((__m128i*)(rnorth + xeast));
        cells = _mm_add_epi8(cells, ne_cells);
        
        w_cells = _mm_loadu_si128((__m128i*)(row + xwest));
        cells = _mm_add_epi8(cells, w_cells);
        e_cells = _mm_loadu_si128((__m128i*)(row + xeast));
        cells = _mm_add_epi8(cells, e_cells);

        s_cells = _mm_loadu_si128((__m128i*)(rsouth + x));
        cells = _mm_add_epi8(cells, s_cells);
        sw_cells = _mm_loadu_si128((__m128i*)(rsouth + xwest));
        cells = _mm_add_epi8(cells, sw_cells);
        se_cells = _mm_loadu_si128((__m128i*)(rsouth + xeast));
        cells = _mm_add_epi8(cells, se_cells);

        r_cells = _mm_loadu_si128((__m128i*)(row + x));
        cells = cpu_simd_16_alive(cells, r_cells);
        _mm_storeu_si128((__m128i*)(buf + irow + x), cells);
    }

    /* Last vector of every row is a special case because the east neighbors 
    wrap around. To access the east neighbors (ne, e, se), the vectors of the 
    north neighbors, the cell themselves, and south neighbors, repectively, are 
    shifted one to the west and the last vector element is replaced by the first
    cell in their respective rows. A right shift is done because the system is 
    little endian. */
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

    cells = cpu_simd_16_alive(cells, r_cells);
    _mm_storeu_si128((__m128i*)(buf + irow + width - 16), cells);
}

/*******************************************************************************
 * CPU SIMD integer type vector
 * 
 * Simulator which processes n cells simultaneously, where n is the size of the
 * integer type specified in the template argument T. This is required because
 * vector intrinsics do not exist for vectors smaller than 64 bits. This does 
 * not execute actual SIMD instructions but rather, it uses hacky operations
 * described below:
 * 
 * 1. Pointer to a cell in grid is reinterpreted and dereferenced as a larger
 *   integer type in order to group multiple cells into a single integer.
 * 2. The integers that represent the neighbors are added together. This is okay
 *   because the maximum sum for every byte is 8, therefore, the sum of one cell
 *   will never produce a carry out that would contaminate the sum of the cell
 *   next to it.
 * 3. Bitwise operations are performed to determine the next state of the cells
 *   in the resulting integer from step 2.
 * 4. The resulting integer from the step 3 is stored to the output buffer 
 *   reinterpreted as a pointer to the larger integer type from before. 
 ******************************************************************************/

/* Determines the next state of each cell in the specified integer type. Since
 * this is not a real SIMD vector type, a bitwise "hack" must be done under the 
 * assumption that each byte in count is between 0 and 8, and each byte in state
 * is either a 0 or 1. */
template <class T>
static inline T cpu_simd_int_alive(T count, T state)
{
    return (state | count) & (count >> 1) & ~(count >> 2) & ~(count >> 3) &
        (T)0x0101010101010101;
}

/* If a whole row can fit in the specified integer type, simply rotate every row
 * one cell to the east or west to the get the west and east neighbors, 
 * respectively.
 * 
 * The _e in the function name stands for equal (width == size of integer). */
template <class T> 
static inline void cpu_simd_int_row_e(char* grid, char* buf, int width, 
    int height, int y, int ynorth, int ysouth)
{
    int vec_len = sizeof(T);
    int irow = y * width;
    int inorth = ynorth * width;
    int isouth = ysouth * width;

    T n_cells = *(T*)(grid + inorth);
    T nw_cells = (n_cells << 8) | (n_cells >> ((vec_len - 1) * 8));
    T ne_cells = (n_cells >> 8) | (n_cells << ((vec_len - 1) * 8));
    T r_cells = *(T*)(grid + irow);
    T w_cells = (r_cells << 8) | (r_cells >> ((vec_len - 1) * 8));
    T e_cells = (r_cells >> 8) | (r_cells << ((vec_len - 1) * 8));
    T s_cells = *(T*)(grid + isouth);
    T sw_cells = (s_cells << 8) | (s_cells >> ((vec_len - 1) * 8));
    T se_cells = (s_cells >> 8) | (s_cells << ((vec_len - 1) * 8));
    T cells = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + 
              sw_cells + se_cells;
    cells = cpu_simd_int_alive<T>(cells, r_cells);
    *(T*)(buf + irow) = cells;
}

/* If the width of the grid is larger than the size of the specified integer 
 * type, the first and last vectors of each row must be handled separately since
 * their west and east neighbors, respectively, wrap around. The middle integers 
 * simply load the addresses offset by one in both directions to access the west
 * and east neighbors.
 * 
 * The _g in the function name stands for greater (width > size of integer). */
template <class T> 
static inline void cpu_simd_int_row_g(char* grid, char* buf, int width, 
    int height, int y, int ynorth, int ysouth)
{
    int vec_len = sizeof(T);
    int irow = y * width;
    int inorth = ynorth * width;
    int isouth = ysouth * width;

    char* rnorth = grid + inorth;
    char* row = grid + irow;
    char* rsouth = grid + isouth;

    /* First vector of every row is a special case because the west neighbors 
    wrap around. */
    T n_cells = *(T*)rnorth;
    T nw_cells = n_cells << 8 | *(rnorth + width - 1);
    T ne_cells = *(T*)(rnorth + 1);
    T r_cells = *(T*)row;
    T w_cells = r_cells << 8 | *(row + width - 1);
    T e_cells = *(T*)(row + 1);
    T s_cells = *(T*)rsouth;
    T sw_cells = s_cells << 8 | *(rsouth + width - 1);
    T se_cells = *(T*)(rsouth + 1);
    T cells = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + 
              sw_cells + se_cells;
    cells = cpu_simd_int_alive<T>(cells, r_cells);
    *(T*)(buf + irow) = cells;

    // Middle vectors
    for (int x = vec_len; x < width - vec_len; x += vec_len) {
        int xeast = x + 1;
        int xwest = x - 1;

        n_cells = *(T*)(rnorth + x);
        nw_cells = *(T*)(rnorth + xwest);
        ne_cells = *(T*)(rnorth + xeast);
        w_cells = *(T*)(row + xwest);
        e_cells = *(T*)(row + xeast);
        s_cells = *(T*)(rsouth + x);
        sw_cells = *(T*)(rsouth + xwest);
        se_cells = *(T*)(rsouth + xeast);
        cells = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + 
                sw_cells + se_cells;
        cells = cpu_simd_int_alive<T>(cells, *(T*)(row + x));
        *(T*)(buf + irow + x) = cells;
    }

    /* Last vector of every row is a special case because the east neighbors 
    wrap around. */
    n_cells = *(T*)(rnorth + width - vec_len);
    nw_cells = *(T*)(rnorth + width - vec_len - 1);
    ne_cells = n_cells >> 8 | ((T)(*rnorth) << ((vec_len - 1) * 8));
    r_cells = *(T*)(row + width - vec_len);
    w_cells = *(T*)(row + width - vec_len - 1);
    e_cells = r_cells >> 8 | ((T)(*row) << ((vec_len - 1) * 8));
    s_cells = *(T*)(rsouth + width - vec_len);
    sw_cells = *(T*)(rsouth + width - vec_len - 1);
    se_cells = s_cells >> 8 | ((T)(*rsouth) << ((vec_len - 1) * 8));
    cells = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + 
            sw_cells + se_cells;
    cells = cpu_simd_int_alive<T>(cells, r_cells);
    *(T*)(buf + irow + width - vec_len) = cells;
}

template <class T>
void sim_cpu_simd_int(char* grid, int width, int height, int gens)
{
    int vec_len = sizeof(T);
    throw_false<std::invalid_argument>(width >= vec_len, "width of the grid "
        "must be at least " + std::to_string(vec_len));
    
    int size = width * height;
    char* buf = new char[size];

    /* Grids with the same width as the size of the specified integer type T 
    are handled separately because they can be optimized even further. See 
    cpu_simd_int_row_e(). */
    if (width == vec_len) {
        for (int i = 0; i < gens; ++i) {
            cpu_simd_int_row_e<T>(grid, buf, width, height, 0, 
                height - 1, 1); // First row
            
            for (int y = 1; y < height - 1; ++y) { // Middle rows
                cpu_simd_int_row_e<T>(grid, buf, width, height, y, y - 1, 
                    y + 1);
            }

            cpu_simd_int_row_e<T>(grid, buf, width, height, height - 1,
                height - 2, 0); // Last row
            swap_ptr(char*, grid, buf);
        }
    }
    else {
        for (int i = 0; i < gens; ++i) {
            cpu_simd_int_row_g<T>(grid, buf, width, height, 0, 
                height - 1, 1); // First row

            for (int y = 1; y < height - 1; ++y) { // Middle rows
                cpu_simd_int_row_g<T>(grid, buf, width, height, y, 
                    y - 1, y + 1);
            }

            cpu_simd_int_row_g<T>(grid, buf, width, height, height - 1,
                height - 2, 0); // Last row
            swap_ptr(char*, grid, buf);
        }
    }

    /* If the number of generations is an odd number, the result would be in buf
    and buf and grid would be flipped. */
    if (gens & 1) { 
        swap_ptr(char*, buf, grid);
        memcpy(grid, buf, size);
    }
    delete[] buf;
}

#endif
