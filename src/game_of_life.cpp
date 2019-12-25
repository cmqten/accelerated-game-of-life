/**
 * game_of_life.cpp
 * 
 * Conway's Game of Life implementation using various techniques to
 * optimize for performance.
 * 
 * Author: Carl Marquez
 * Created on: June 15, 2018
 */ 
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>

#include <game_of_life.hpp>
#include <util.hpp>

// Minimum dimension of 3 so every cell has 8 neighbors, max dimension of 16384
// to not consume too much memory, can be increased if system has more.
const int min_dim = 3;
const int max_dim = 16384;

// For error logging
const std::string min_dim_str = std::to_string(min_dim);
const std::string max_dim_str = std::to_string(max_dim);

/* Generate a random world. */
char* generate_random_world(int width, int height, int percent_alive)
{
    if (percent_alive < 0 || percent_alive > 100) {
        throw std::invalid_argument("percent_alive must be between 0 and 100");
    }
    if (width < min_dim || width > max_dim) {
        throw std::invalid_argument("width must be between " + min_dim_str + " and " + max_dim_str);
    }
    if (height < min_dim || height > max_dim) {
        throw std::invalid_argument("height must be between " + min_dim_str + " and " + max_dim_str);
    }
    int size = width * height;
    char* world = (char*)aligned_alloc(64, size);

    srand(time(nullptr));
    for (int i = 0; i < size; i++) {
        if (rand() % 100 < percent_alive) {
            world[i] = 1;
        }
        else {
            world[i] = 0;
        }
    }
    return world;
}

static void benchmark(int width, int height, int percent_alive, int gens)
{
    int size = width * height;

    // Create one world for each simulator
    std::unique_ptr<char[]> world_seq(generate_random_world(width, height, percent_alive));

    std::unique_ptr<char[]> world_simd((char*)aligned_alloc(64, size));
    memcpy(world_simd.get(), world_seq.get(), size);

    std::unique_ptr<char[]> world_omp((char*)aligned_alloc(64, size));
    memcpy(world_omp.get(), world_seq.get(), size);

    // Simulate every copy of the world for the same number generations on
    // different simulators. The result must be the same for all.
    my_timer timer;
    timer.start();
    cpu_seq(world_seq.get(), width, height, gens);
    double seq_time = timer.stop();

    timer.start();
    cpu_simd(world_simd.get(), width, height, gens);
    double simd_time = timer.stop();

    timer.start();
    cpu_omp(world_omp.get(), width, height, gens);
    double omp_time = timer.stop();

    printf("+------------------------------+\n");
    printf("| Implementation | Runtime (s) |\n");
    printf("|------------------------------|\n");
    printf("| CPU Sequential | %11.2f |\n", seq_time);
    printf("| CPU SIMD 1T    | %11.2f |\n", simd_time);
    printf("| CPU OpenMP     | %11.2f |\n", omp_time);
    printf("+------------------------------+\n\n");

    if (memcmp(world_seq.get(), world_simd.get(), size)) {
        std::cout << "CPU SIMD is not equal to the reference implementation" << std::endl;
    }
    else if (memcmp(world_seq.get(), world_omp.get(), size)) {
        std::cout << "CPU OpenMP is not equal to the reference implementation" << std::endl;
    }
    else {
        std::cout << "All implementations are equal" << std::endl;
    }
}

int main(int argc, char** argv)
{
    benchmark(256, 256, 50, 100000);
    return 0;
}
