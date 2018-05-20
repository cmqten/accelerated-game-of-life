/**
 * sim_cpu_simd.cpp
 * 
 * Optimized data parallel implementation of Conway's Game of Life using SIMD 
 * operations. Uses SSE2 and SSSE3 extensions.
 * 
 * Due to some of the operations performed in this implementation such as 
 * vector extensions and unaligned memory accesses, this is not portable and is
 * only guaranteed to work on an x86_64 system with the extensions specified 
 * above.
 * 
 * Formerly called game_of_life_cpu_simd.cpp.
 * 
 * Author: Carl Marquez
 * Created on: May 7, 2018
 */
#include <cstring>
#include <x86intrin.h>
#include "sim_cpu_simd.hpp"
#include "sim.hpp"
#include "util.hpp"

/* CPU SIMD 128-bit vector SSE2/SSSE3
 *
 * Calculates the next states of 16 cells in a vector. */
inline __m128i alive_simd_16(__m128i count, __m128i state)
{
    return _mm_and_si128(
        _mm_or_si128(
            _mm_cmpeq_epi8(count, _mm_set1_epi8(3)), 
            _mm_and_si128(state, _mm_cmpeq_epi8(count, _mm_set1_epi8(2)))
        ), 
        _mm_set1_epi8(1)
    );
}

/* CPU SIMD 128-bit vector SSE2/SSSE3
 * 
 * If a whole row can fit in a single vector, simply rotate every row one cell 
 * to the east or west to the get the west and east neighbors, respectively.
 * 
 * The _e in the function name stands for equal (width == size of vector). */
inline void simulate_row_simd_16_e(char* grid, char* buf, int width, int height, 
    int y, int ynorth, int ysouth)
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

    cells = alive_simd_16(cells, r_cells);
    _mm_store_si128((__m128i*)(buf + irow), cells);
}

/* CPU SIMD 128-bit vector SSE2/SSSE3
 * 
 * If the width of the grid is larger than 16, the first and last vectors of 
 * each row must be handled separately since their west and east neighbors, 
 * respectively, wrap around. The middle vectors simply load the addresses 
 * offset by one in both directions to access the west and east neighbors.
 * 
 * The _g in the function name stands for greater (width > size of vector). */
inline void simulate_row_simd_16_g(char* grid, char* buf, int width, int height,
    int y, int ynorth, int ysouth)
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

    cells = alive_simd_16(cells, r_cells);
    _mm_storeu_si128((__m128i*)(buf + irow), cells);

    // Middle vectors
    for (int x = 16; x < width - 16; x += 16) {
        int ieast = x + 1;
        int iwest = x - 1;

        /* Initialize cells to the north neighbors to clear the cells from the 
        previous iteration. */
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

    cells = alive_simd_16(cells, r_cells);
    _mm_storeu_si128((__m128i*)(buf + irow + width - 16), cells);
}

/* CPU SIMD 128-bit vector SSE2/SSSE3
 * 
 * Simulator which processes 16 cells simultaneously using SIMD operations. */
void sim_cpu_simd_16(char* grid, int width, int height, int gens)
{
    throw_false<std::invalid_argument>(width >= 16, "width of the grid must be"
        " at least 16");

    int size = width * height;
    char* buf = new char[size];

    /* Grids with width of 16 are handled separately because they can be 
    optimized even further. See simulate_row_simd_16_e(). */
    if (width == 16) {
        for (int i = 0; i < gens; ++i) {
            // First row
            simulate_row_simd_16_e(grid, buf, width, height, 0, height - 1, 1);

            for (int y = 1; y < height - 1; ++y) // Middle rows
                simulate_row_simd_16_e(grid, buf, width, height, y, y-1, y+1);

            simulate_row_simd_16_e(grid, buf, width, height, height - 1, 
                height - 2, 0); // Last row
            swap_ptr(char*, grid, buf);
        }
    }
    else {
        for (int i = 0; i < gens; ++i) {
            // First row
            simulate_row_simd_16_g(grid, buf, width, height, 0, height - 1, 1);

            for (int y = 1; y < height - 1; ++y) // Middle rows
                simulate_row_simd_16_g(grid, buf, width, height, y, y-1, y+1);

            simulate_row_simd_16_g(grid, buf, width, height, height - 1,
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

/* Game of Life CPU SIMD
 * 
 * Different width ranges are handled separately to maximize vector size for 
 * maximum parallelism without overrunning a row (vector size > width). */ 
void sim_cpu_simd(char* grid, int width, int height, int gens)
{
    if (width >= 16) 
        sim_cpu_simd_16(grid, width, height, gens);
    else if (width >= 8)
        sim_cpu_simd_int<uint64_t>(grid, width, height, gens);
    else if (width >= 4)
        sim_cpu_simd_int<uint32_t>(grid, width, height, gens);
    else if (width >= 2)
        sim_cpu_simd_int<uint16_t>(grid, width, height, gens);
    else
        sim_cpu_simd_int<uint8_t>(grid, width, height, gens);
}