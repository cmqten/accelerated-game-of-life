/**
 * Functions for loading and saving a board to a pbm file.
 */
#include <cstring>
#include <fstream>
#include <iostream>
#include "board.hpp"

/**
 * Converts an array of '0' and '1' char to an array of 0 and 1 int. The
 * precondition is that it is an array of only '0' and '1', so subtracting '0'
 * is sufficient. Returns true if the operation is successful, false otherwise.
 */
bool ascii_to_int(char* board, int width, int height)
{
    if (!board_width_height_okay(board, width, height)) return false;
    int size = width * height;
    for (int i = 0; i < size; ++i) board[i] -= '0';
    return true;
}

/**
 * Converts an array of 0 and 1 int to an array of '0' and '1' char. The
 * precondition is that it is an array of only 0 and 1, so adding '0' is 
 * sufficient. Returns true if the operation is successful, false otherwise.
 */
bool int_to_ascii(char* board, int width, int height)
{
    if (!board_width_height_okay(board, width, height)) return false;
    int size = width * height;
    for (int i = 0; i < size; ++i) board[i] += '0';
    return true;
}

/**
 * Generates a random board of the specified size and percent. Returns nullptr
 * if any of the arguments are invalid, returns a generated board otherwise.
 */
char* random_board(int width, int height, int percent) 
{
    int size = width * height;
    char* board = new char[size];
    srand(time(nullptr));
    for (int i = 0; i < size; ++i) board[i] = ((rand() % 100) < percent) + '0'; 
    return board;
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
 * successful, nullptr otherwise. The char array returned must be freed using
 * delete[].
 * 
 * For more information about the pbm format, go to: 
 * http://netpbm.sourceforge.net/doc/pbm.html
 */
char* load_board(int* width, int* height, std::string filename)
{
    std::ifstream file;
    file.open(filename);
    if (!file) {
        std::cerr << "Error: file doesn't exist or is unreadable" << std::endl;
        return nullptr;
    }

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
        std::cerr << "Error: width overflow/underflow \n";
        return nullptr;
    }
    if (!in_range(width_l, MIN_WIDTH, MAX_WIDTH)) {
        std::cerr << "Error: width must be between " << MIN_WIDTH << " and ";
        std::cerr << MAX_WIDTH << std::endl;
        return nullptr;
    }

    int height_l = strtol(remain, nullptr, 10);
    if (errno) {
        std::cerr << "Error: height overflow/underflow" << std::endl;
        return nullptr;
    }
    if (!in_range(height_l, MIN_HEIGHT, MAX_HEIGHT)) {
        std::cerr << "Error: height must be between " << MIN_HEIGHT << " and ";
        std::cerr << MAX_HEIGHT << std::endl;
        return nullptr;
    }

    // The third line is a string of '0' and '1' that represents the board
    int size = width_l * height_l;
    char* board = new char[size];
    file.read(board, size);

    // width and height are only set if the whole operation is successful
    if (width) *width = width_l;
    if (height) *height = height_l;
    return board;
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
 * as a long string of '0' and '1', with no withspaces in between. 
 * 
 * For more information about the pbm format, go to: 
 * http://netpbm.sourceforge.net/doc/pbm.html
 */
bool save_board(char* board, int width, int height, std::string filename) 
{
    if (!board_width_height_okay(board, width, height)) return false;
    int size = width * height;
    std::ofstream file;
    file.open(filename);

    if (!file) {
        std::cerr << "Error: file is unwriteable" << std::endl;
        return false;
    }
    file << "P1" << std::endl;
    file << width << " " << height << std::endl;
    file.write(board, size);
    file.close();
    return true;
}
