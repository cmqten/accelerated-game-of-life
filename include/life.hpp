/**
 * life.hpp
 * 
 * Different implementations of Conway's Game of Life.
 * 
 * Author: Carl Marquez
 * Created on: May 5, 2018
 */
#ifndef __LIFE_HPP__
#define __LIFE_HPP__

/* Optimized CPU sequential implementation */
void life_cpu_seq(char* grid, int width, int height, int gens);

/* Optimized single thread CPU SIMD implementation */ 
void life_cpu_simd(char* grid, int width, int height, int gens);

/* Optimized multithread CPU SIMD implementation using OpenMP */
void life_cpu_omp(char* grid, int width, int height, int gens);

/* Optimized GPU SIMD implementation using OpenCL */
void life_gpu_ocl(char* grid, int width, int height, int gens);

#endif
