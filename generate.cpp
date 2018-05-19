/**
 * generate.cpp
 * 
 * Generates a random Game of Life grid according to the specified population
 * percentage and size then saves it as a pbm file.
 * 
 * Author: Carl Marquez
 * Created on: April 13, 2018
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

    game_of_life gol = game_of_life::create_random(width, height, percent);
    gol.save(argv[4]); // implicit conversion from char[] to string
    return 0;
}
