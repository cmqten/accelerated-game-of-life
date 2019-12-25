/**
 * life_cpu_omp.cpp
 * 
 * Extends the SIMD optimization by running multiple threads using OpenMP.
 * 
 * Author: Carl Marquez
 * Created on: May 19, 2018
 */
#include <omp.h>
#include <stdexcept>

#include <cpu_simd.hpp>

const int cache_line = 64; // Depends on system, most are 64

/* Processes 16 cells simultaneously, multithreaded. */
static void cpu_omp_simd_16(char* grid, int width, int height, int gens, int threads)
{
    if (width < 16) {
        throw std::invalid_argument("width must be at least 16");
    }
    int size = width * height;
    char* buf = new char[size];

    // Threads get at least one cache line of cells to prevent false sharing. 
    int rows_per_thread = (height + threads - 1) / threads;
    int cells_per_thread = rows_per_thread * width;
    if (cells_per_thread < cache_line) {
        rows_per_thread = (cache_line + width - 1) / width;
    }

    // Removes unused threads.
    threads = (height + rows_per_thread - 1) / rows_per_thread;

    // Width of 16 handled separately because it can be optimized further.
    if (width == 16) {
        #pragma omp parallel num_threads(threads) default(none) \
        shared(height, gens, rows_per_thread) firstprivate(grid, buf)
        {
            int tid = omp_get_thread_num();
            int y_start = tid * rows_per_thread;
            int y_end = y_start + rows_per_thread;

            for (int i = 0; i < gens; i++) {
                for (int y = y_start; y < y_end && y < height; y++) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_16_row_16w(grid, buf, y, y_north, y_south);
                }
                swap_ptr((void**)&grid, (void**)&buf);
                #pragma omp barrier
            }
        }
    }
    else {
        #pragma omp parallel num_threads(threads) default(none) \
        shared(width, height, gens, rows_per_thread) firstprivate(grid, buf)
        {
            int tid = omp_get_thread_num();
            int y_start = tid * rows_per_thread;
            int y_end = y_start + rows_per_thread;

            for (int i = 0; i < gens; i++) {
                for (int y = y_start; y < y_end && y < height; y++) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_16_row(grid, buf, width, y, y_north, y_south);
                }
                swap_ptr((void**)&grid, (void**)&buf);
                #pragma omp barrier
            }
        }
    }

    // If number of generations is odd, the result is in buf, so copy to grid.
    if (gens % 2) {
        memcpy(grid, buf, size);
    }
    delete[] buf;
}

/* Processes n cells simultaneously, where n is the size of T, multithreaded. */
template <class T>
static void cpu_omp_simd_int(char* grid, int width, int height, int gens, int threads)
{
    int vec_len = sizeof(T);
    if (width < vec_len) {
        throw std::invalid_argument("width must be at least " + std::to_string(vec_len));
    }
    int size = width * height;
    char* buf = new char[size];

    // Threads get at least one cache line of cells to prevent false sharing. 
    int rows_per_thread = (height + threads - 1) / threads;
    int cells_per_thread = rows_per_thread * width;
    if (cells_per_thread < cache_line) {
        rows_per_thread = (cache_line + width - 1) / width;
    }

    // Removes unused threads.
    threads = (height + rows_per_thread - 1) / rows_per_thread;

    // Width of same size as T handled separately because it can be optimized 
    // further.
    if (width == sizeof(T)) {
        #pragma omp parallel num_threads(threads) default(none) \
        shared(height, gens, rows_per_thread) firstprivate(grid, buf)
        {
            int tid = omp_get_thread_num();
            int y_start = tid * rows_per_thread;
            int y_end = y_start + rows_per_thread;

            for (int i = 0; i < gens; i++) {
                for (int y = y_start; y < y_end && y < height; y++) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_int_row_intw<T>(grid, buf, y, y_north, y_south);
                }
                swap_ptr((void**)&grid, (void**)&buf);
                #pragma omp barrier
            }
        }
    }
    else {
        #pragma omp parallel num_threads(threads) default(none) \
        shared(width, height, gens, rows_per_thread) firstprivate(grid, buf)
        {
            int tid = omp_get_thread_num();
            int y_start = tid * rows_per_thread;
            int y_end = y_start + rows_per_thread;

            for (int i = 0; i < gens; i++) {
                for (int y = y_start; y < y_end && y < height; y++) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_int_row<T>(grid, buf, width, y, y_north, y_south);
                }
                swap_ptr((void**)&grid, (void**)&buf);
                #pragma omp barrier
            }
        }
    }

    // If number of generations is odd, the result is in buf, so copy to grid.
    if (gens % 2) {
        memcpy(grid, buf, size);
    }
    delete[] buf;
}

void cpu_omp(char* grid, int width, int height, int gens)
{
    int threads = omp_get_num_procs();
    if (width >= 16) {
        cpu_omp_simd_16(grid, width, height, gens, threads);
    }
    else if (width >= 8) {
        cpu_omp_simd_int<uint64_t>(grid, width, height, gens, threads);
    }
    else if (width >= 4) {
        cpu_omp_simd_int<uint32_t>(grid, width, height, gens, threads);
    }
    else if (width >= 2) {
        cpu_omp_simd_int<uint16_t>(grid, width, height, gens, threads);
    }
    else {
        cpu_omp_simd_int<uint8_t>(grid, width, height, gens, threads);
    }
}