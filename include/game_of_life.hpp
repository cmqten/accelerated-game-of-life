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

/* CPU sequential */
void cpu_seq(char* grid, int width, int height, int gens);

/* Single-threaded CPU SIMD */ 
void cpu_simd(char* grid, int width, int height, int gens);

/* Multi-threaded CPU SIMD with OpenMP */
void cpu_omp(char* grid, int width, int height, int gens);

#endif
