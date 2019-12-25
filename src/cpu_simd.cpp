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

/* Processes 16 cells simultaneously. */
void cpu_simd_16(char* grid, int width, int height, int gens)
{
    if (width < 16) {
        throw std::invalid_argument("width must be at least 16");
    }
    int size = width * height;
    char* buf = new char[size];

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
    delete[] buf;
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
