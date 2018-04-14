/**
 * Functions for loading and saving a board to a pbm file.
 */
#include <experimental/filesystem>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include "board.hpp"

/**
 * Helper function to check if a number is in range of two other numbers, 
 * inclusive.
 */
static inline bool in_range(int val, int min, int max)
{
    return val >= min && val <= max;
}

/**
 * Converts an array of '0' and '1' char to an array of 0 and 1 int. The
 * precondition is that it is an array of only '0' and '1', so subtracting '0'
 * is sufficient. Returns true if the operation is successful, false otherwise.
 */
bool ascii_to_int(char* board, int width, int height)
{
    if (!board) {
        std::cerr << "Error: board can't be null" << std::endl;
        return false;
    }
    if (in_range(width, MIN_WIDTH, MAX_WIDTH)) {
        std::cerr << "Error: width must be between " << MIN_WIDTH << " and ";
        std::cerr << MAX_WIDTH << std::endl;
        return false;
    }
    if (in_range(height, MIN_HEIGHT, MAX_HEIGHT)) {
        std::cerr << "Error: height must be between " << MIN_HEIGHT << " and ";
        std::cerr << MAX_HEIGHT << std::endl;
        return false;
    }
    int size = width * height;
    for (int i = 0; i < size; ++i) {
        board[i] -= '0';
    }
    return true;
}

/**
 * Converts an array of 0 and 1 int to an array of '0' and '1' char. The
 * precondition is that it is an array of only 0 and 1, so adding '0' is 
 * sufficient. Returns true if the operation is successful, false otherwise.
 */
bool int_to_ascii(char* board, int width, int height)
{
    if (!board) {
        std::cerr << "Error: board can't be null" << std::endl;
        return false;
    }
    if (in_range(width, MIN_WIDTH, MAX_WIDTH)) {
        std::cerr << "Error: width must be between " << MIN_WIDTH << " and ";
        std::cerr << MAX_WIDTH << std::endl;
        return false;
    }
    if (in_range(height, MIN_HEIGHT, MAX_HEIGHT)) {
        std::cerr << "Error: height must be between " << MIN_HEIGHT << " and ";
        std::cerr << MAX_HEIGHT << std::endl;
        return false;
    }
    int size = width * height;
    for (int i = 0; i < size; ++i) {
        board[i] += '0';
    }
    return true;
}

/**
 * Saves board as a plain pbm file with the following format: 
 * 
 * P1
 * <width> <height>
 * <board>
 * 
 * There are no whitespaces before each line, and each line ends with a newline
 * character. The board must be an array of '0' and '1' characters. It is saved 
 * as a long string of '0' and '1', with no withspaces in between. For more
 * information about the pbm format, go to: 
 * http://netpbm.sourceforge.net/doc/pbm.html
 */
void save_board(char* board, int width, int height, std::string filename) 
{
    if (!board) {
        std::cerr << "Error: board can't be null" << std::endl;
        return;
    }
    if (width < MIN_WIDTH || width > MAX_WIDTH) {
        std::cerr << "Error: width must be between " << MIN_WIDTH << " and ";
        std::cerr << MAX_WIDTH << std::endl;
        return;
    }
    if (height < MIN_HEIGHT || height > MAX_HEIGHT) {
        std::cerr << "Error: height must be between " << MIN_HEIGHT << " and ";
        std::cerr << MAX_HEIGHT << std::endl;
        return;
    }
    int size = width * height;
    std::ofstream file;
    file.open(filename);
    file << "P1" << std::endl;
    file << width << " " << height << std::endl;
    file.write(board, size);
    file.close();
}

/**
 * Loads board from a plain pbm file with the same format as:
 * 
 * P1
 * <width> <height>
 * <board>
 * 
 * There are no whitespaces before each line, and each line ends with a newline
 * character. This format is a subset of the netpbm specifications. For 
 * simplicity of checking, any format that does not follow above will be 
 * considered invalid by this function, even though it may follow the 
 * netpbm specifications. save_board() is guaranteed to save a board that can be 
 * loaded by load_board(). Returns the board as a char array if the operation is 
 * successful, nullptr otherwise.
 */
char* load_board(int* width, int* height, std::string filename)
{
    if (!std::experimental::filesystem::exists(filename)) {
        std::cerr << "Error: file doesn't exist" << std::endl;
        return nullptr;
    }
    std::ifstream file;
    file.open(filename);

    // Checks for the magic number "P1"
    std::string line;
    std::getline(file, line);
    if (line.compare(0, 2, "P1")) {
        std::cerr << "Error: not a plain pbm file" << std::endl;
        return nullptr;
    }

    // The second line must be the dimensions
    std::getline(file, line);
    char* remain;
    int width_l = strtol(line.c_str(), &remain, 10);
    if (errno) {
        std::cerr << "Error: width decimal literal overflow/underflow \n";
        return nullptr;
    }
    if (width_l < 1 || width_l > 32768) {
        std::cerr << "Error: width must be a decimal literal between";
        std::cerr << " 1 and 32768" << std::endl;
        return nullptr;
    }

    int height_l = strtol(remain, nullptr, 10);
    if (errno) {
        std::cerr << "Error: height decimal literal overflow/underflow \n";
        return nullptr;
    }
    if (height_l < 1 || height_l > 32768) {
        std::cerr << "Error: height must be a decimal literal between";
        std::cerr << " 1 and 32768" << std::endl;
        return nullptr;
    }

    if (width) *width = width_l;
    if (height) *height = height_l;

    // The third line us a string of '0' and '1' that represents the board
    int size = width_l * height_l;
    char* board = (char*)calloc(sizeof(char), size);
    if (!board) {
        std::cerr << "Error: calloc error" << std::endl;
        return nullptr;
    }
    file.read(board, size);
    return board;
}