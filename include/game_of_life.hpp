/**
 * game_of_life.hpp
 * 
 * Conway's Game of Life implementation using various techniques to
 * optimize for performance.
 * 
 * Author: Carl Marquez
 * Created on: May 5, 2018
 */
#ifndef __GAME_OF_LIFE_HPP__
#define __GAME_OF_LIFE_HPP__

typedef void (*cpu_sim_t)(char*, int, int, int);

/* CPU sequential */
void cpu_seq(char* grid, int width, int height, int gens);

/* Single-threaded CPU SIMD */ 
void cpu_simd(char* grid, int width, int height, int gens);

/* Multi-threaded CPU SIMD with OpenMP */
void cpu_omp(char* grid, int width, int height, int gens);

/* GPU with OpenCL */
void gpu_ocl(char* grid, int width, int height, int gens, double* compute_time = nullptr, 
    double* transfer_in_time = nullptr, double* transfer_out_time = nullptr);

#endif
