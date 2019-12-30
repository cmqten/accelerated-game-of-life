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

/* Generates a random world. */
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

/* Simulates game of life on CPU and returns the runtime in ms. */
double run_game_of_life_cpu(cpu_sim_t func, char* world, int width, int height, int gens)
{
    my_timer timer;
    timer.start();
    func(world, width, height, gens);
    return timer.stop();
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

    std::unique_ptr<char[]> world_gpu((char*)aligned_alloc(64, size));
    memcpy(world_gpu.get(), world_seq.get(), size);

    // Simulate every copy of the world for the same number generations on
    // different simulators. The result must be the same for all.
    double seq_time = run_game_of_life_cpu(cpu_seq, world_seq.get(), width, height, gens);
    double simd_time = run_game_of_life_cpu(cpu_simd, world_simd.get(), width, height, gens);
    double omp_time = run_game_of_life_cpu(cpu_omp, world_omp.get(), width, height, gens);
    double ocl_time;
    gpu_ocl(world_gpu.get(), width, height, gens, &ocl_time);

    // Print runtimes
    std::cout << "Size: " << width << " x " << height << std::endl;
    std::cout << "Generations: " << gens << std::endl;
    printf("+-----------------------------------------+\n");
    printf("| Simulator      | Compute (ms) | Speedup |\n");
    printf("|-------------------------------|---------|\n");
    printf("| CPU Sequential | %12.2f | %6.2fx |\n", seq_time, 1.0);
    printf("| CPU SIMD 1T    | %12.2f | %6.2fx |\n", simd_time, seq_time / simd_time);
    printf("| CPU OpenMP     | %12.2f | %6.2fx |\n", omp_time, seq_time / omp_time);
    printf("| GPU OpenCL     | %12.2f | %6.2fx |\n", ocl_time, seq_time / ocl_time);
    printf("+-----------------------------------------+\n\n");

    if (memcmp(world_seq.get(), world_simd.get(), size)) {
        std::cerr << "CPU SIMD is not equal to the reference implementation" << std::endl;
    }
    else if (memcmp(world_seq.get(), world_omp.get(), size)) {
        std::cerr << "CPU OpenMP is not equal to the reference implementation" << std::endl;
    }
    else if (memcmp(world_gpu.get(), world_omp.get(), size)) {
        std::cerr << "CPU OpenCL is not equal to the reference implementation" << std::endl;
    }
}

int main(int argc, char** argv)
{
    int gens = 100000;
    //benchmark(3, 1024, 50, gens);
    benchmark(4, 1024, 50, gens);
    //benchmark(6, 1024, 50, gens);
    benchmark(8, 1024, 50, gens);
    //benchmark(9, 1024, 50, gens);
    //benchmark(15, 1024, 50, gens);
    benchmark(16, 1024, 50, gens);
    benchmark(25, 1024, 50, gens);
    benchmark(32, 1024, 50, gens);
    benchmark(253, 256, 50, gens);
    benchmark(256, 256, 50, gens);
    benchmark(1024, 1024, 50, 10000);
    return 0;
}
