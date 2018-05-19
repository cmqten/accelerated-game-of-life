/**
 * game_of_life_sim.hpp
 * 
 * Different implementations of Conway's Game of Life.
 * 
 * Author: Carl Marquez
 * Created on: May 5, 2018
 */
#ifndef __GAME_OF_LIFE_SIM_HPP__
#define __GAME_OF_LIFE_SIM_HPP__

void game_of_life_cpu_sequential(char* grid, int width, int height, int gens);

void game_of_life_cpu_simd(char* grid, int width, int height, int gens);

void game_of_life_cpu_threads(char* grid, int width, int height, int gens);

void game_of_life_cpu_omp(char* grid, int width, int height, int gens);

void game_of_life_gpu_opencl(char* grid, int width, int height, int gens);

#endif
