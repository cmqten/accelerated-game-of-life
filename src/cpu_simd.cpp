/**
 * life_cpu_simd.cpp
 * 
 * Implementation of Conway's Game of Life using SIMD operations. Not portable, 
 * only guaranteed to work on an x86_64 system with SSE2 and SSSE3 extensions,
 * and unaligned memory access support.
 * 
 * Author: Carl Marquez
 * Created on: May 7, 2018
 */
#include <cstring>
#include <stdexcept>

#include <cpu_simd.hpp>
#include <game_of_life.hpp>
#include <util.hpp>

/* Processes n cells simultaneously, where n is the size of T. */
template <class T>
void cpu_simd_int(char* grid, int width, int height, int gens)
{
    int vec_len = sizeof(T);
    if (width < vec_len) {
        throw std::invalid_argument("width must be at least " + std::to_string(vec_len));
    }
    int size = width * height;
    char* buf = (char*)aligned_alloc(64, size);

    /* Grids with the same width as the size of the specified integer type T 
    are handled separately because they can be optimized even further. See 
    cpu_simd_int_row_e(). */
    if (width == 4) {
        for (int i = 0; i < gens; i++) {
            cpu_simd_16_row_4w(grid, buf, 0, height - 1, 4); 
            for (int y = 4; y < height - 4; y += 4) {
                cpu_simd_16_row_4w(grid, buf, y, y - 1, y + 4);
            }
            cpu_simd_16_row_4w(grid, buf, height - 4, height - 5, 0); 
            swap_ptr((void**)&grid, (void**)&buf);
        }
    }
    else if (width == vec_len) {
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
    free(buf);
}

/* Processes 16 cells simultaneously. */
void cpu_simd_16(char* grid, int width, int height, int gens)
{
    if (width < 16) {
        throw std::invalid_argument("width must be at least 16");
    }
    int size = width * height;
    char* buf = (char*)aligned_alloc(64, size);

    // Width of 16 handled separately because it can be optimized further.
    if (width == 16) {
        for (int i = 0; i < gens; i++) {
            // First and last rows are outside of the loop to not have to check
            // for north and south neighbor bounds.
            cpu_simd_16_row_16w(grid, buf, 0, height - 1, 1); 
            for (int y = 1; y < height - 1; y++) { 
                cpu_simd_16_row_16w(grid, buf, y, y - 1, y + 1);
            }
            cpu_simd_16_row_16w(grid, buf, height - 1, height - 2, 0); 
            swap_ptr((void**)&grid, (void**)&buf);
        }
    }
    else {
        for (int i = 0; i < gens; i++) {
            // First and last rows are outside of the loop to not have to check
            // for north and south neighbor bounds.
            cpu_simd_16_row(grid, buf, width, 0, height - 1, 1);
            for (int y = 1; y < height - 1; y++) {
                cpu_simd_16_row(grid, buf, width, y, y - 1, y + 1);
            }
            cpu_simd_16_row(grid, buf, width, height - 1, height - 2, 0); 
            swap_ptr((void**)&grid, (void**)&buf);
        }
    }

    // If number of generations is odd, the result is in buf, so swap with grid. 
    if (gens % 2) { 
        swap_ptr((void**)&grid, (void**)&buf);
        memcpy(grid, buf, size);
    }
    free(buf);
}

/* Game of Life CPU SIMD

Different width ranges are handled separately to maximize vector size for 
maximum parallelism without overrunning a row (vector size > width). */ 
void cpu_simd(char* grid, int width, int height, int gens)
{
    if (width >= 16) {
        cpu_simd_16(grid, width, height, gens);
    }
    else if (width >= 8) {
        cpu_simd_int<uint64_t>(grid, width, height, gens);
    }
    else if (width >= 4) {
        cpu_simd_int<uint32_t>(grid, width, height, gens);
    }
    else if (width >= 2) {
        cpu_simd_int<uint16_t>(grid, width, height, gens);
    }
    else {
        cpu_simd_int<uint8_t>(grid, width, height, gens);
    }
}
