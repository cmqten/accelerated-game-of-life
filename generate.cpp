/**
 * Generates a random game of life board according to the specified population
 * percentage and size and saves it as a pbm file.
 */
#include <cstring>
#include <fstream>
#include <iostream>
#include "board.hpp"

/**
 * Prints an error message and exits with a non-zero status code.
 */
static inline void exit_error(std::string msg) 
{
    std::cerr << msg << std::endl;
    exit(1);
}

int main(int argc, char** argv) 
{
    if (argc != 5) exit_error("Usage: generate WIDTH HEIGHT PERCENT FILENAME");

    /**
     * Limits the maximum dimensions to 32768 x 32768 so that there is no
     * risk of overflowing when calculating the size of the image. Also, don't
     * want the image to be too big because the system may run out of memory.
     */
    int width = strtol(argv[1], NULL, 10);
    if (errno) exit_error("Error: width overflow/underflow");
    if (!in_range(width, MIN_WIDTH, MAX_WIDTH)) 
        exit_error("Error: width must be between " + std::to_string(MIN_WIDTH) 
            + " and " + std::to_string(MAX_WIDTH));

    int height = strtol(argv[2], NULL, 10);
    if (errno) exit_error("Error: height overflow/underflow");
    if (!in_range(height, MIN_HEIGHT, MAX_HEIGHT)) 
        exit_error("Error: height must be between " + std::to_string(MIN_HEIGHT) 
            + " and " + std::to_string(MAX_HEIGHT));

    int percent = strtol(argv[3], NULL, 10);
    if (errno) exit_error("Error: percent overflow/underflow");
    if (!in_range(percent, 1, 100)) 
        exit_error("Error: percent must be between 1 and 100");

    std::string filename(argv[4]);
    char* board = random_board(width, height, percent);
    save_board(board, width, height, filename);
    delete[] board;
    return 0;
}
