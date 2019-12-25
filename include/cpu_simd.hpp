/**
 * cpu_simd.hpp
 * 
 * Implementation of Conway's Game of Life using SIMD operations. Not portable, 
 * only guaranteed to work on an x86_64 system with SSE2 and SSSE3 extensions,
 * and unaligned memory access support.
 * 
 * Author: Carl Marquez
 * Created on: May 19, 2018
 */
#ifndef __CPU_SIMD_HPP__
#define __CPU_SIMD_HPP__

#include <cstring>
#include <x86intrin.h>
#include <util.hpp>

/*******************************************************************************
 * CPU SIMD 128-bit vector SSE2/SSSE3
 * 
 * Processes 16 cells simultaneously.
 ******************************************************************************/

void cpu_simd_16(char* grid, int width, int height, int gens);

/* Calculates the next states of 16 cells in a vector. */
static inline __m128i cpu_simd_16_alive(__m128i cells, __m128i neighbors_count)
{
    __m128i has_3_neighbors = _mm_cmpeq_epi8(neighbors_count, _mm_set1_epi8(3));
    __m128i has_2_neighbors = _mm_cmpeq_epi8(neighbors_count, _mm_set1_epi8(2));
    __m128i alive_has_2_neighbors = _mm_and_si128(cells, has_2_neighbors);
    cells = _mm_or_si128(has_3_neighbors, alive_has_2_neighbors);
    return _mm_and_si128(cells, _mm_set1_epi8(1));
}

/* Processes rows with exactly 16 width. */
static inline void cpu_simd_16_row_16w(char* grid, char* buf, int y, int y_north, int y_south)
{
    int width = 16;
    int i_row = y * width;
    int i_north = y_north * width;
    int i_south = y_south * width;

    // East/west, northeast/northwest, southeast/southwest cells are rotations
    // of current cells, north, south cells, respectively.
    __m128i cells = _mm_load_si128((__m128i*)(grid + i_row));
    __m128i n_cells = _mm_load_si128((__m128i*)(grid + i_north));
    __m128i ne_cells = _mm_alignr_epi8(n_cells, n_cells, 1);
    __m128i nw_cells = _mm_alignr_epi8(n_cells, n_cells, 15);
    __m128i e_cells = _mm_alignr_epi8(cells, cells, 1);
    __m128i w_cells = _mm_alignr_epi8(cells, cells, 15);
    __m128i s_cells = _mm_load_si128((__m128i*)(grid + i_south));
    __m128i se_cells = _mm_alignr_epi8(s_cells, s_cells, 1);
    __m128i sw_cells = _mm_alignr_epi8(s_cells, s_cells, 15);

    __m128i neighbors_count = n_cells;
    neighbors_count = _mm_add_epi8(neighbors_count, ne_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, nw_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, e_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, w_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, s_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, se_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, sw_cells);

    cells = cpu_simd_16_alive(cells, neighbors_count);
    _mm_store_si128((__m128i*)(buf + i_row), cells);
}

/* Processes a row with greater than 16 width. */
static inline void cpu_simd_16_row(char* grid, char* buf, int width, int y, int y_north, int y_south)
{
    int i_row = y * width;
    int i_north = y_north * width;
    int i_south = y_south * width;

    // Pointers to the start of the north, current, and south rows
    char* p_north = grid + i_north;
    char* p_row = grid + i_row;
    char* p_south = grid + i_south;

    // First vector is a special case because the west neighbors wrap around. 
    // To access the west, northwest, southwest cells, the current cells, north, 
    // south cells, repectively, are left shifted one and the first vector 
    // element is replaced by the last cell in their respective rows.
    __m128i neighbors_count;
    __m128i cells = _mm_loadu_si128((__m128i*)(p_row));
    __m128i n_cells = _mm_loadu_si128((__m128i*)(p_north));
    __m128i ne_cells = _mm_loadu_si128((__m128i*)(p_north + 1));
    __m128i nw_cells = _mm_alignr_epi8(n_cells, _mm_set1_epi8(p_north[width - 1]), 15);
    __m128i e_cells = _mm_loadu_si128((__m128i*)(p_row + 1));
    __m128i w_cells = _mm_alignr_epi8(cells, _mm_set1_epi8(p_row[width - 1]), 15);
    __m128i s_cells = _mm_loadu_si128((__m128i*)(p_south));
    __m128i se_cells = _mm_loadu_si128((__m128i*)(p_south + 1));
    __m128i sw_cells = _mm_alignr_epi8(s_cells, _mm_set1_epi8(p_south[width - 1]), 15);

    neighbors_count = n_cells;
    neighbors_count = _mm_add_epi8(neighbors_count, ne_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, nw_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, e_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, w_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, s_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, se_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, sw_cells);

    cells = cpu_simd_16_alive(cells, neighbors_count);
    _mm_storeu_si128((__m128i*)(buf + i_row), cells);

    // Middle vectors
    for (int x = 16; x < width - 16; x += 16) {
        int x_east = x + 1;
        int x_west = x - 1;

        cells = _mm_loadu_si128((__m128i*)(p_row + x));
        n_cells = _mm_loadu_si128((__m128i*)(p_north + x));
        nw_cells = _mm_loadu_si128((__m128i*)(p_north + x_west));
        ne_cells = _mm_loadu_si128((__m128i*)(p_north + x_east));
        w_cells = _mm_loadu_si128((__m128i*)(p_row + x_west));
        e_cells = _mm_loadu_si128((__m128i*)(p_row + x_east));
        s_cells = _mm_loadu_si128((__m128i*)(p_south + x));
        sw_cells = _mm_loadu_si128((__m128i*)(p_south + x_west));
        se_cells = _mm_loadu_si128((__m128i*)(p_south + x_east));

        neighbors_count = n_cells;
        neighbors_count = _mm_add_epi8(neighbors_count, ne_cells);
        neighbors_count = _mm_add_epi8(neighbors_count, nw_cells);
        neighbors_count = _mm_add_epi8(neighbors_count, e_cells);
        neighbors_count = _mm_add_epi8(neighbors_count, w_cells);
        neighbors_count = _mm_add_epi8(neighbors_count, s_cells);
        neighbors_count = _mm_add_epi8(neighbors_count, se_cells);
        neighbors_count = _mm_add_epi8(neighbors_count, sw_cells);

        cells = cpu_simd_16_alive(cells, neighbors_count);
        _mm_storeu_si128((__m128i*)(buf + i_row + x), cells);
    }

    // Last vector is a special case because the east neighbors wrap around. 
    // To access the east, northeast, southeast cells, the current cells, north, 
    // south cells, repectively, are right shifted one and the last vector 
    // element is replaced by the first cell in their respective rows.
    cells = _mm_loadu_si128((__m128i*)(p_row + width - 16));
    n_cells = _mm_loadu_si128((__m128i*)(p_north + width - 16));
    nw_cells = _mm_loadu_si128((__m128i*)(p_north + width - 17));
    ne_cells = _mm_alignr_epi8(_mm_set1_epi8(*p_north), n_cells, 1);
    w_cells = _mm_loadu_si128((__m128i*)(p_row + width - 17));
    e_cells = _mm_alignr_epi8(_mm_set1_epi8(*p_row), cells, 1);
    s_cells = _mm_loadu_si128((__m128i*)(p_south + width - 16));
    sw_cells = _mm_loadu_si128((__m128i*)(p_south + width - 17));
    se_cells = _mm_alignr_epi8(_mm_set1_epi8(*p_south), s_cells, 1);

    neighbors_count = n_cells;
    neighbors_count = _mm_add_epi8(neighbors_count, nw_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, ne_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, w_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, e_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, s_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, sw_cells);
    neighbors_count = _mm_add_epi8(neighbors_count, se_cells);

    cells = cpu_simd_16_alive(cells, neighbors_count);
    _mm_storeu_si128((__m128i*)(buf + i_row + width - 16), cells);
}

/*******************************************************************************
 * CPU SIMD integer type vector
 * 
 * Processes n cells simultaneously, where n is the size of the integer type 
 * specified in the template argument T. No vector intrinsics for vector sizes 
 * less than 16 bytes (as far as I know), so the following are done to mimic
 * vector operations:
 * 
 * 1. Pointer to a cell in grid is reinterpreted and dereferenced as a larger
 *    integer type to group multiple cells. Every byte in integer is a cell.
 * 2. The integers that represent the neighbors are added together. Maximum sum 
 *    for every byte is 8, so sum of one byte will never produce a carry out 
 *    that would contaminate the sum of the byte after it.
 * 3. Bitwise operations are performed to determine the next state of the cells.
 * 4. The result is stored to the output buffer reinterpreted as a pointer to 
 *    the larger integer type from before. 
 ******************************************************************************/

/* Calculates the next states of cells in an integer. */
template <class T>
static inline T cpu_simd_int_alive(T cells, T neighbors_count)
{
    // Bitwise hack I came up with in 3rd year undergrad for a game of life
    // assignment. Assumption is each byte/cell is 0 or 1, which represents 
    // dead or alive, respectively.
    return (cells | neighbors_count) & (neighbors_count >> 1) & ~(neighbors_count >> 2) & 
            ~(neighbors_count >> 3) & (T)0x0101010101010101;
}

/* Processes rows with width the same size as integer type. */
template <class T> 
static inline void cpu_simd_int_row_intw(char* grid, char* buf, int y, int y_north, int y_south)
{
    int vec_len = sizeof(T);
    int width = vec_len;
    int i_row = y * width;
    int i_north = y_north * width;
    int i_south = y_south * width;

    // East/west, northeast/northwest, southeast/southwest cells are rotations
    // of current cells, north, south cells, respectively.
    T cells = *(T*)(grid + i_row);
    T n_cells = *(T*)(grid + i_north);
    T nw_cells = (n_cells << 8) | (n_cells >> ((vec_len - 1) * 8));
    T ne_cells = (n_cells >> 8) | (n_cells << ((vec_len - 1) * 8));
    T w_cells = (cells << 8) | (cells >> ((vec_len - 1) * 8));
    T e_cells = (cells >> 8) | (cells << ((vec_len - 1) * 8));
    T s_cells = *(T*)(grid + i_south);
    T sw_cells = (s_cells << 8) | (s_cells >> ((vec_len - 1) * 8));
    T se_cells = (s_cells >> 8) | (s_cells << ((vec_len - 1) * 8));

    T neighbors_count = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + sw_cells + se_cells;
    cells = cpu_simd_int_alive<T>(cells, neighbors_count);
    *(T*)(buf + i_row) = cells;
}

/* Processes rows with width size greater than integer type. */
template <class T> 
static inline void cpu_simd_int_row(char* grid, char* buf, int width, int y, int y_north, int y_south)
{
    int vec_len = sizeof(T);
    int i_row = y * width;
    int i_north = y_north * width;
    int i_south = y_south * width;

    char* p_north = grid + i_north;
    char* p_row = grid + i_row;
    char* p_south = grid + i_south;

    // First vector is a special case because the west neighbors wrap around. 
    // To access the west, northwest, southwest cells, the current cells, north, 
    // south cells, repectively, are left shifted one and the first vector 
    // element is replaced by the last cell in their respective rows.
    T cells = *(T*)p_row;
    T n_cells = *(T*)p_north;
    T nw_cells = n_cells << 8 | *(p_north + width - 1);
    T ne_cells = *(T*)(p_north + 1);
    T w_cells = cells << 8 | *(p_row + width - 1);
    T e_cells = *(T*)(p_row + 1);
    T s_cells = *(T*)p_south;
    T sw_cells = s_cells << 8 | *(p_south + width - 1);
    T se_cells = *(T*)(p_south + 1);

    T neighbors_count = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + sw_cells + se_cells;
    cells = cpu_simd_int_alive<T>(cells, neighbors_count);
    *(T*)(buf + i_row) = cells;

    // Middle vectors
    for (int x = vec_len; x < width - vec_len; x += vec_len) {
        int x_east = x + 1;
        int x_west = x - 1;
        
        cells = *(T*)(p_row + x);
        n_cells = *(T*)(p_north + x);
        nw_cells = *(T*)(p_north + x_west);
        ne_cells = *(T*)(p_north + x_east);
        w_cells = *(T*)(p_row + x_west);
        e_cells = *(T*)(p_row + x_east);
        s_cells = *(T*)(p_south + x);
        sw_cells = *(T*)(p_south + x_west);
        se_cells = *(T*)(p_south + x_east);

        neighbors_count = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + sw_cells + se_cells;
        cells = cpu_simd_int_alive<T>(cells, neighbors_count);
        *(T*)(buf + i_row + x) = cells;
    }

    // Last vector is a special case because the east neighbors wrap around. 
    // To access the east, northeast, southeast cells, the current cells, north, 
    // south cells, repectively, are right shifted one and the last vector 
    // element is replaced by the first cell in their respective rows.
    n_cells = *(T*)(p_north + width - vec_len);
    nw_cells = *(T*)(p_north + width - vec_len - 1);
    ne_cells = n_cells >> 8 | ((T)(*p_north) << ((vec_len - 1) * 8));
    cells = *(T*)(p_row + width - vec_len);
    w_cells = *(T*)(p_row + width - vec_len - 1);
    e_cells = cells >> 8 | ((T)(*p_row) << ((vec_len - 1) * 8));
    s_cells = *(T*)(p_south + width - vec_len);
    sw_cells = *(T*)(p_south + width - vec_len - 1);
    se_cells = s_cells >> 8 | ((T)(*p_south) << ((vec_len - 1) * 8));

    neighbors_count = n_cells + nw_cells + ne_cells + w_cells + e_cells + s_cells + sw_cells + se_cells;
    cells = cpu_simd_int_alive<T>(cells, neighbors_count);
    *(T*)(buf + i_row + width - vec_len) = cells;
}

/* Processes n cells simultaneously, where n is the size of T. */
template <class T>
void cpu_simd_int(char* grid, int width, int height, int gens)
{
    int vec_len = sizeof(T);
    if (width < vec_len) {
        throw std::invalid_argument("width must be at least " + std::to_string(vec_len));
    }
    int size = width * height;
    char* buf = new char[size];

    /* Grids with the same width as the size of the specified integer type T 
    are handled separately because they can be optimized even further. See 
    cpu_simd_int_row_e(). */
    if (width == vec_len) {
        for (int i = 0; i < gens; i++) {
            cpu_simd_int_row_intw<T>(grid, buf, 0, height - 1, 1); 
            for (int y = 1; y < height - 1; y++) {
                cpu_simd_int_row_intw<T>(grid, buf, y, y - 1, y + 1);
            }
            cpu_simd_int_row_intw<T>(grid, buf, height - 1, height - 2, 0); 
            swap_ptr((void**)&grid, (void**)&buf);
        }
    }
    else {
        for (int i = 0; i < gens; i++) {
            cpu_simd_int_row<T>(grid, buf, width, 0, height - 1, 1);
            for (int y = 1; y < height - 1; y++) {
                cpu_simd_int_row<T>(grid, buf, width, y, y - 1, y + 1);
            }
            cpu_simd_int_row<T>(grid, buf, width, height - 1, height - 2, 0); 
            swap_ptr((void**)&grid, (void**)&buf);
        }
    }

    // If number of generations is odd, the result is in buf, so swap with grid. 
    if (gens % 2) { 
        swap_ptr((void**)&buf, (void**)&grid);
        memcpy(grid, buf, size);
    }
    delete[] buf;
}

#endif
