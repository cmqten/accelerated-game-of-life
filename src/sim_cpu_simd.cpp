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
#include <sim_cpu_simd.hpp>
#include <sim.hpp>
#include <util.hpp>

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
    optimized even further. See cpu_simd_16_row_e(). */
    if (width == 16) {
        for (int i = 0; i < gens; ++i) {
            // First row
            cpu_simd_16_row_e(grid, buf, width, height, 0, height - 1, 1);

            for (int y = 1; y < height - 1; ++y) // Middle rows
                cpu_simd_16_row_e(grid, buf, width, height, y, y - 1, y + 1);

            // Last row
            cpu_simd_16_row_e(grid, buf, width, height, height-1, height-2, 0); 
            swap_ptr(char*, grid, buf);
        }
    }
    else {
        for (int i = 0; i < gens; ++i) {
            // First row
            cpu_simd_16_row_g(grid, buf, width, height, 0, height - 1, 1);

            for (int y = 1; y < height - 1; ++y) // Middle rows
                cpu_simd_16_row_g(grid, buf, width, height, y, y - 1, y + 1);
            
            // Last row
            cpu_simd_16_row_g(grid, buf, width, height, height-1, height-2, 0); 
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