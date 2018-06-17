/**
 * life_cpu_omp.cpp
 * 
 * Extends the SIMD optimization by running multiple threads using OpenMP.
 * 
 * Author: Carl Marquez
 * Created on: May 19, 2018
 */
#include <omp.h>
#include <life_cpu_simd.hpp>

int threads = 4; // Modify as needed
int cache_line = 64; // Depends on system, most are 64

/* OpenMP multithreaded version of sim_cpu_simd_16() */
static void life_cpu_omp_simd_16(char* grid, int width, int height, int gens)
{
    throw_false<std::invalid_argument>(width >= 16, "width of the grid must be"
        " at least 16");

    int size = width * height;
    char* buf = new char[size];
    int rows_per_thread = (height + threads - 1) / threads;
    int cells_per_thread = rows_per_thread * width;

    /* Guarantees that every thread gets at least one cache line worth of cells
    to prevent false sharing. */
    if (cells_per_thread < cache_line)
        rows_per_thread = (cache_line + width - 1) / width;

    /* Guarantees that only the exact amount of threads needed are spawned. 
    Prevents cases such as 3 rows and 4 threads, 1 thread is not doing work. */
    threads = (height + rows_per_thread - 1) / rows_per_thread;

    /* Grids with width of 16 are handled separately because they can be 
    optimized even further. See cpu_simd_16_row_e(). */
    if (width == 16) {
        #pragma omp parallel num_threads(threads) default(none) \
        shared(width, height, gens, rows_per_thread) firstprivate(grid, buf)
        {
            int tid = omp_get_thread_num();
            int y_start = tid * rows_per_thread;
            int y_end = y_start + rows_per_thread;

            for (int i = 0; i < gens; ++i) {
                for (int y = y_start; y < y_end && y < height; ++y) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_16_row_e(grid, buf, width, y, y_north, y_south);
                }
                swap_ptr(char*, grid, buf);
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

            for (int i = 0; i < gens; ++i) {
                for (int y = y_start; y < y_end && y < height; ++y) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_16_row_g(grid, buf, width, y, y_north, y_south);
                }
                swap_ptr(char*, grid, buf);
                #pragma omp barrier
            }
        }
    }

    /* If generations is an odd number, the result will be in buf so copy it 
    back to grid. */
    if (gens & 1) memcpy(grid, buf, size);
    delete[] buf;
}

/* OpenMP multithreaded version of sim_cpu_simd_int() */
template <class T>
static void life_cpu_omp_simd_int(char* grid, int width, int height, int gens)
{
    throw_false<std::invalid_argument>(width >= (int)sizeof(T), "width of the "
        "grid must be at least " + std::to_string(sizeof(T)));
    
    int size = width * height;
    char* buf = new char[size];
    int rows_per_thread = (height + threads - 1) / threads;
    int cells_per_thread = rows_per_thread * width;

    /* Guarantees that every thread gets at least one cache line worth of cells
    to prevent false sharing */
    if (cells_per_thread < cache_line)
        rows_per_thread = (cache_line + width - 1) / width;

    /* Guarantees that only the exact amount of threads needed are spawned. 
    Prevents cases such as 3 rows and 4 threads, 1 thread is not doing work. */
    threads = (height + rows_per_thread - 1) / rows_per_thread;

    /* Grids with the same width as the size of the specified integer type T 
    are handled separately because they can be optimized even further. See 
    cpu_simd_int_row_e(). */
    if (width == sizeof(T)) {
        #pragma omp parallel num_threads(threads) default(none) \
        shared(width, height, gens, rows_per_thread) firstprivate(grid, buf)
        {
            int tid = omp_get_thread_num();
            int y_start = tid * rows_per_thread;
            int y_end = y_start + rows_per_thread;

            for (int i = 0; i < gens; ++i) {
                for (int y = y_start; y < y_end && y < height; ++y) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_int_row_e<T>(grid, buf, width, y, y_north,
                        y_south);
                }
                swap_ptr(char*, grid, buf);
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

            for (int i = 0; i < gens; ++i) {
                for (int y = y_start; y < y_end && y < height; ++y) {
                    int y_north = y ? y - 1 : height - 1;
                    int y_south = y == height - 1 ? 0 : y + 1;
                    cpu_simd_int_row_g<T>(grid, buf, width, y, y_north,
                        y_south);
                }
                swap_ptr(char*, grid, buf);
                #pragma omp barrier
            }
        }
    }

    /* If generations is an odd number, the result will be in buf so copy it 
    back to grid. */
    if (gens & 1) memcpy(grid, buf, size);
    delete[] buf;
}

void life_cpu_omp(char* grid, int width, int height, int gens)
{
    if (width >= 16) 
        life_cpu_omp_simd_16(grid, width, height, gens);
    else if (width >= 8)
        life_cpu_omp_simd_int<uint64_t>(grid, width, height, gens);
    else if (width >= 4)
        life_cpu_omp_simd_int<uint32_t>(grid, width, height, gens);
    else if (width >= 2)
        life_cpu_omp_simd_int<uint16_t>(grid, width, height, gens);
    else
        life_cpu_omp_simd_int<uint8_t>(grid, width, height, gens);
}