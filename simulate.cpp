#include <iostream>
#include <sys/sysinfo.h>
#include "cell_world.hpp"
#include "sim.hpp"

int main(int argc, char** argv)
{
    int gens = 1000;
    cell_world gol_seq = cell_world::create_random(15, 4190, 50,
        sim_cpu_sequential);
    cell_world gol_simd = cell_world::create_from_existing(gol_seq);
    gol_simd.simulator = sim_cpu_simd;
    cell_world gol_omp = cell_world::create_from_existing(gol_seq);
    gol_omp.simulator = sim_cpu_omp;

    std::cout << "CPU sequential: " << gol_seq.simulate(gens) << " seconds" 
              << std::endl;
    std::cout << "CPU SIMD: " << gol_simd.simulate(gens) << " seconds" 
              << std::endl;
    std::cout << "CPU OpenMP: " << gol_omp.simulate(gens) << " seconds" 
              << std::endl;

    if (gol_simd == gol_seq && gol_seq == gol_omp) 
        std::cout << "equal" << std::endl;
    else 
        std::cout << "not equal" << std::endl;

    return 0;
}