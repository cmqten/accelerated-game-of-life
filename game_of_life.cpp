#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "game_of_life.hpp"

/**
 * A copy of buf is made to ensure that the board is allocated correctly (using
 * new) since board is deallocated using delete[] in the destructor of 
 * game_of_life.
 */
game_of_life* game_of_life::create_from_buffer(char* buf, int width, int height)
{
    throw_board_null<std::invalid_argument>(buf);
    throw_width_out_of_range<std::invalid_argument>(width);
    throw_height_out_of_range<std::invalid_argument>(height);

    int size = width * height;
    char* board = new char[size];
    memcpy(board, buf, size);
    return new game_of_life(board, width, height);
}

/**
 * The file must be a plain pbm file with the specified format in the header
 * file game_of_life.hpp.
 */
game_of_life* game_of_life::create_from_file(const std::string& filename) 
{
    std::ifstream file;
    file.open(filename);
    throw_true<std::runtime_error>(!file, "file cannot be opened for reading");

    // Checks for the magic number "P1"
    std::string line;
    std::getline(file, line);
    throw_non_zero<std::runtime_error>(line.compare("P1"), "not a pbm file");

    // The second line is the dimensions
    char* remain;
    std::getline(file, line);
    int width = strtol(line.c_str(), &remain, 10);
    throw_non_zero<std::overflow_error>(errno, "width overflow/underflow");
    throw_width_out_of_range<std::out_of_range>(width);

    int height = strtol(remain, nullptr, 10);
    throw_non_zero<std::overflow_error>(errno, "height overflow/underflow");
    throw_height_out_of_range<std::out_of_range>(height);

    // The third line is a string of '0' and '1' that represents the board
    int size = width * height;
    char* board = new char[size];
    file.read(board, size);   
    file.close();
    for (int i = 0; i < size; ++i) board[i] -= '0'; // ascii to int
    return new game_of_life(board, width, height);
}

game_of_life* game_of_life::create_random(int width, int height, int percent)
{
    throw_width_out_of_range<std::invalid_argument>(width);
    throw_height_out_of_range<std::invalid_argument>(height);
    throw_false<std::invalid_argument>(in_range(percent, 1, 100), 
        "percent out of range of 1 and 100");

    int size = width * height;
    char* board = new char[size];
    srand(time(nullptr));
    for (int i = 0; i < size; ++i) board[i] = ((rand() % 100) < percent);
    return new game_of_life(board, width, height);
}

game_of_life::game_of_life(char* board, int width, int height) : 
    board(board), width(width), height(height), size(width * height) 
{ 
}

game_of_life::~game_of_life() 
{
    delete[] board;
}

bool game_of_life::operator==(const game_of_life& other) const
{
    return width == other.width && height == other.height &&
        !memcmp(board, other.board, size);
}

bool game_of_life::operator!=(const game_of_life& other) const
{
    return !operator==(other);
}

void game_of_life::save(const std::string& filename) const
{
    std::ofstream file;
    file.open(filename);
    throw_true<std::runtime_error>(!file, "file cannot be opened for writing");

    char* buf = new char[size];
    memcpy(buf, board, size); // Don't want to modify the original board
    for (int i = 0; i < size; ++i) buf[i] += '0';

    file << "P1\n" << width << " " << height << std::endl;
    file.write(buf, size);
    file.close();
    delete[] buf;
}
