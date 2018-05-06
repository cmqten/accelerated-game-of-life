/**
 * Generates a random game of life board according to the specified population
 * percentage and size and saves it as a pbm file.
 */
#include <cstring>
#include <fstream>
#include <iostream>
#include "game_of_life.hpp"
#include "util.hpp"

int main(int argc, char** argv) 
{
    if (argc != 5) {
        std::cerr << "Usage: generate WIDTH HEIGHT PERCENT FILENAME"
            << std::endl;
        return 1;
    }

    int width = strtol(argv[1], NULL, 10);
    throw_non_zero<std::overflow_error>(errno, "width overflow/underflow");

    int height = strtol(argv[2], NULL, 10);
    throw_non_zero<std::overflow_error>(errno, "height overflow/underflow");

    int percent = strtol(argv[3], NULL, 10);
    throw_non_zero<std::overflow_error>(errno, "percent overflow/underflow");

    game_of_life* board1 = game_of_life::create_random(width, height, percent);
    board1->save(argv[4]); // implicit conversion from char[] to string
    delete board1;
    return 0;
}
