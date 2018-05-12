#include <iostream>
#include "game_of_life.hpp"
#include "simulators.hpp"

int main(int argc, char** argv)
{
    int gens = 10000;
    game_of_life* gol_seq = game_of_life::create_random(15, 1024, 50,
        game_of_life_cpu_sequential);
    game_of_life* gol_simd = new game_of_life(*gol_seq);
    gol_simd->simulator = game_of_life_cpu_simd;
    
    double duration = gol_seq->simulate(gens);
    std::cout << "CPU sequential: " << duration << " seconds" << std::endl;
    duration = gol_simd->simulate(gens);
    std::cout << "CPU SIMD: " << duration << " seconds" << std::endl;

    if ((*gol_simd) == (*gol_seq)) std::cout << "equal" << std::endl;
    else std::cout << "not equal" << std::endl;

    delete gol_seq;
    delete gol_simd;
    return 0;
}