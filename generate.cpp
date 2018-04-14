/**
 * Generates a random game of life board according to the specified population
 * percentage and size and saves it as a pbm file.
 */
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "board.hpp"

/**
 * Prints an error message and exits with a non-zero status code.
 */
static inline void exit_error(std::string msg) 
{
    std::cerr << "Error: " << msg << std::endl;
    exit(1);
}

/**
 * Generates a random board of the specified size and percent. Returns nullptr 
 * if an error occurs and prints the error to stderr.
 */
char* random_board(int width, int height, int percent) 
{
    int size = width * height;
    char* board = (char*)calloc(sizeof(char), size);
    if (!board) {
        std::cerr << "Out of memory" << std::endl;
        return nullptr;
    }

    srand(time(nullptr));
    for (int i = 0; i < size; ++i) {
        board[i] = ((rand() % 100) < percent) + '0'; 
    }

    return board;
}

int main(int argc, char** argv) 
{
    if (argc != 5) exit_error("Usage: generate WIDTH HEIGHT PERCENT FILENAME");

    /**
     * Limits the maximum dimensions to 32768 x 32768 so that there is no
     * risk of overflowing when calculating the size of the image. Also, don't
     * want the image to be too big because the GPU may run out of memory.
     */
    int width = strtol(argv[1], NULL, 10);
    if (errno) 
        exit_error("Width decimal literal overflow/underflow");
    if (width < 1 || width > 32768) 
        exit_error("Width must be a decimal literal between 1 and 32768");

    int height = strtol(argv[2], NULL, 10);
    if (errno) 
        exit_error("Height decimal literal overflow/underflow");
    if (height < 1 || height > 32768) 
        exit_error("Height must be a decimal literal between 1 and 32768");

    int percent = strtol(argv[3], NULL, 10);
    if (errno) 
        exit_error("Percent decimal literal overflow/underflow");
    if (percent < 1 || percent > 100) 
        exit_error("Percent must be between 1 and 100");

    std::string filename(argv[4]);

    char* board = random_board(width, height, percent);
    if (!board) return 1;
    save_board(board, width, height, filename);
    free(board);

    return 0;
}
