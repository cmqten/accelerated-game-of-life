/**
 * sim.hpp
 * 
 * Different implementations of Conway's Game of Life.
 * 
 * Formerly called game_of_life_sim.hpp
 * 
 * Author: Carl Marquez
 * Created on: May 5, 2018
 */
#ifndef __SIM_HPP__
#define __SIM_HPP__

/* Optimized CPU sequential implementation */
void sim_cpu_sequential(char* grid, int width, int height, int gens);

/* Optimized single thread CPU SIMD implementation */ 
void sim_cpu_simd(char* grid, int width, int height, int gens);

/* Optimized multithread CPU SIMD implementation using OpenMP */
void sim_cpu_omp(char* grid, int width, int height, int gens);

/* Optimized GPU SIMD implementation using OpenCL */
void sim_gpu_ocl(char* grid, int width, int height, int gens);

#endif
