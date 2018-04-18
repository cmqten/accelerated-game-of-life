/**
 * Functions for loading and saving a board to a pbm file.
 */
#include <cstring>
#include <fstream>
#include <iostream>
#include "board.hpp"

/**
 * Converts an array of '0' and '1' char to an array of 0 and 1 int. 
 */
static inline void ascii_to_int(char* board, int width, int height)
{
    int size = width * height;
    for (int i = 0; i < size; ++i) board[i] -= '0';
}

/**
 * Converts an array of 0 and 1 int to an array of '0' and '1' char. 
 */
static inline void int_to_ascii(char* board, int width, int height)
{
    int size = width * height;
    for (int i = 0; i < size; ++i) board[i] += '0';
}

/**
 * Generates a random board of the specified size and percent. Returns an array
 * of 0 and 1 integers which represents a board.
 */
char* random_board(int width, int height, int percent) 
{
    int size = width * height;
    char* board = new char[size];
    srand(time(nullptr));
    for (int i = 0; i < size; ++i) board[i] = ((rand() % 100) < percent); 
    return board;
}

/**
 * Loads board from a plain pbm file with the format specified in board.hpp.
 * Guaranteed to return a valid board because an exception will be thrown if
 * any of the operations preceding the return fails. The board returned is an
 * array of 0 and 1 integers.
 */
char* load_board(int* width, int* height, std::string filename)
{
    std::ifstream file;
    file.open(filename);
    if (!file) throw std::runtime_error("File does not exist or is unreadable");

    // Checks for the magic number "P1"
    std::string line;
    std::getline(file, line);
    if (line.compare(0, 2, "P1")) throw std::runtime_error("Not a pbm file");

    // The second line must be the dimensions
    char* remain;
    std::getline(file, line);
    int width_l = strtol(line.c_str(), &remain, 10);
    if (errno) throw std::overflow_error("Width overflow/underflow");
        
    if (!in_range(width_l, MIN_WIDTH, MAX_WIDTH))
        throw std::range_error("Width must be between " + 
            std::to_string(MIN_WIDTH) + " and " + std::to_string(MAX_WIDTH));

    int height_l = strtol(remain, nullptr, 10);
    if (errno) throw std::overflow_error("Height overflow/underflow");

    if (!in_range(height_l, MIN_HEIGHT, MAX_HEIGHT)) 
        throw std::range_error("Height must be between " + 
            std::to_string(MIN_HEIGHT) + " and " + std::to_string(MAX_HEIGHT));

    // The third line is a string of '0' and '1' that represents the board
    int size = width_l * height_l;
    char* board = new char[size];
    file.read(board, size);
    ascii_to_int(board, width_l, height_l);

    // width and height are only set if the whole operation is successful
    if (width) *width = width_l;
    if (height) *height = height_l;
    
    file.close();
    return board;
}

/**
 * Saves board as a plain pbm file with the format specified in board.hpp. Any
 * board saved by the function is guaranteed to be able to be loaded by 
 * load_board(). The input board must be an array of 0 and 1 integers.
 */
void save_board(char* board, int width, int height, std::string filename) 
{
    if (!board_width_height_okay(board, width, height))
        throw std::logic_error("Null board or dimensions out of range");

    int size = width * height;
    char* buf = new char[size]; 
    memcpy(buf, board, size); // Don't want to modify the original board
    int_to_ascii(buf, width, height);

    std::ofstream file;
    file.open(filename);
    if (!file) throw std::runtime_error("File does not exist or is unwritable");

    file << "P1\n" << width << " " << height << std::endl;
    file.write(buf, size);
    file.close();
    delete[] buf;
}
