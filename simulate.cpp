#include <iostream>
#include "game_of_life.hpp"
#include "simulators.hpp"

int main(int argc, char** argv)
{
    int gens = 100;
    game_of_life* b = game_of_life::create_random(1024, 1024, 50,
        game_of_life_cpu_sequential);
    double duration = b->simulate(gens);
    std::cout << duration << " seconds" << std::endl;
    delete b;
    return 0;
}