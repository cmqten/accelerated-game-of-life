/**
 * game_of_life_cpu_simd.hppp
 * 
 * Optimized data parallel implementation of Conway's Game of Life using SIMD 
 * operations. Uses SSE2 and SSSE3 extensions.
 * 
 * Due to some of the operations performed in this implementation such as 
 * vector extensions and unaligned memory accesses, this is not portable and is
 * only guaranteed to work on an x86_64 system with the extensions specified 
 * above.
 * 
 * Author: Carl Marquez
 * Created on: May 19, 2018
 */
#ifndef __GAME_OF_LIFE_CPU_SIMD_HPP__
#define __GAME_OF_LIFE_CPU_SIMD_HPP__

#include <cstring>
#include <x86intrin.h>
#include "game_of_life_sim.hpp"
#include "util.hpp"


/*******************************************************************************
 * CPU SIMD 128-bit vector SSE2/SSSE3
 * 
 * An optimized single-thread implementation which uses SIMD operations to
 * process cells in parallel. 
 ******************************************************************************/

/* Calculates the next states of 16 cells in a vector. */
inline __m128i alive_simd_16(__m128i count, __m128i state);

/* Simulates a row of cells if the width of the grid is exactly 16. */
inline void simulate_row_simd_16_e(char* grid, char* buf, int width, int height, 
    int y, int ynorth, int ysouth);

/* Simulates a row of cells if the width of the grid is greater than 16. */
inline void simulate_row_simd_16_g(char* grid, char* buf, int width, int height,
    int y, int ynorth, int ysouth);

void game_of_life_cpu_simd_16(char* grid, int width, int height, int gens);


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
inline T alive_simd_int(T count, T state)
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
inline void simulate_row_simd_int_e(char* grid, char* buf, int width, 
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
    cells = alive_simd_int<T>(cells, r_cells);
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
inline void simulate_row_simd_int_g(char* grid, char* buf, int width, 
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
    cells = alive_simd_int<T>(cells, r_cells);
    *(T*)(buf + irow) = cells;

    // Middle vectors
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
        cells = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + 
                sw_cells + se_cells;
        cells = alive_simd_int<T>(cells, *(T*)(row + x));
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
    cells = alive_simd_int<T>(cells, r_cells);
    *(T*)(buf + irow + width - vec_len) = cells;
}

template <class T>
void game_of_life_cpu_simd_int(char* grid, int width, int height, int gens)
{
    int vec_len = sizeof(T);
    throw_false<std::invalid_argument>(width >= vec_len, "width of the grid "
        "must be at least " + std::to_string(vec_len));
    
    int size = width * height;
    char* buf = new char[size];

    /* Grids with the same width as the size of the specified integer type T 
    are handled separately because they can be optimized even further. See 
    simulate_row_simd_int_e(). */
    if (width == vec_len) {
        for (int i = 0; i < gens; ++i) {
            simulate_row_simd_int_e<T>(grid, buf, width, height, 0, 
                height - 1, 1); // First row
            
            for (int y = 1; y < height - 1; ++y) { // Middle rows
                simulate_row_simd_int_e<T>(grid, buf, width, height, y, y - 1, 
                    y + 1);
            }

            simulate_row_simd_int_e<T>(grid, buf, width, height, height - 1,
                height - 2, 0); // Last row
            swap_ptr(char*, grid, buf);
        }
    }
    else {
        for (int i = 0; i < gens; ++i) {
            simulate_row_simd_int_g<T>(grid, buf, width, height, 0, 
                height - 1, 1); // First row

            for (int y = 1; y < height - 1; ++y) { // Middle rows
                simulate_row_simd_int_g<T>(grid, buf, width, height, y, 
                    y - 1, y + 1);
            }

            simulate_row_simd_int_g<T>(grid, buf, width, height, height - 1,
                height - 2, 0); // Last row
            swap_ptr(char*, grid, buf);
        }
    }

    if (gens & 1) { 
        swap_ptr(char*, buf, grid);
        memcpy(grid, buf, size);
    }
    delete[] buf;
}

#endif
