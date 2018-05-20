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

void sim_cpu_sequential(char* grid, int width, int height, int gens);

void sim_cpu_simd(char* grid, int width, int height, int gens);

void sim_cpu_threads(char* grid, int width, int height, int gens);

void sim_cpu_omp(char* grid, int width, int height, int gens);

void sim_gpu_opencl(char* grid, int width, int height, int gens);

#endif
