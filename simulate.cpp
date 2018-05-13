#include <iostream>
#include "game_of_life.hpp"
#include "simulators.hpp"

int main(int argc, char** argv)
{
    int gens = 10000;
    game_of_life gol_seq = game_of_life::create_random(64, 1024, 50,
        game_of_life_cpu_sequential);
    game_of_life gol_simd = game_of_life::create_from_existing(gol_seq);
    gol_simd.simulator = game_of_life_cpu_simd;

    std::cout << "CPU sequential: " << gol_seq.simulate(gens) << " seconds" 
              << std::endl;
    std::cout << "CPU SIMD: " << gol_simd.simulate(gens) << " seconds" 
              << std::endl;

    if (gol_simd == gol_seq) std::cout << "equal" << std::endl;
    else std::cout << "not equal" << std::endl;

    return 0;
}