/**
 * Different algorithms for simulating Conway's game of life
 */
#ifndef __GAME_OF_LIFE_SIM_HPP__
#define __GAME_OF_LIFE_SIM_HPP__

#include <stdint.h>

void game_of_life_cpu_sequential(char* board, int width, int height, int gens);

void game_of_life_cpu_simd(char* board, int width, int height, int gens);

void game_of_life_cpu_threads(char* board, int width, int height, int gens);

void game_of_life_cpu_omp(char* board, int width, int height, int gens);

void game_of_life_gpu_opencl(char* board, int width, int height, int gens);

#endif
