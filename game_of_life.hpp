/**
 * Conway's game of life implementation using C++. A board is represented using
 * a char array. The char array may be generated randomly by a method in the 
 * game_of_life class, or loaded from a plain pbm file. When loading a file, the 
 * file must have the following format:
 * 
 * P1
 * <width> <height>
 * <board>
 * 
 * The first line must contain the string "P1" at the first index immediately 
 * followed by a newline character. The second line must have the width in 
 * decimal at the first index, followed by a single space and the height in 
 * decimal, immediately followed by a newline character. The last line
 * must contain a string of '0' and '1' with no spaces in between them. This 
 * format is a subset of the plain pbm specifications, so some pbm files that
 * conform to the specifications may not be loaded for simplicity of checking
 * and loading. For more information about the netpbm specifications, visit:
 * http://netpbm.sourceforge.net/doc/pbm.html
 */
#ifndef __GAME_OF_LIFE_HPP__
#define __GAME_OF_LIFE_HPP__

#include <string>
#include "util.hpp"

/**
 * The board dimensions must be within the following ranges. This is to prevent
 * the system from running out of memory.
 */
const int min_width = 1;
const int max_width = 32768;
const int min_height = 1;
const int max_height = 32768;

/**
 * Encapsulates the buffer that represents the game of life board, as well as 
 * its dimensions and overall size. Provides methods for generating a board,
 * simulating, and loading from/saving to file.
 */
class game_of_life 
{
private:
    explicit game_of_life(char* board, int width, int height);

public:
    const char* board;
    const int width;
    const int height;
    const int size;

    ~game_of_life();

    bool operator==(const game_of_life& other) const;

    bool operator!=(const game_of_life& other) const;

    /**
     * Saves the board as the specified filename. Throws a runtime_error if the
     * file cannot be opened for writing. 
     */
    void save(const std::string& filename) const;

    /**
     * Creates a board from a pre-existing buffer of 0 and 1 integers. The 
     * buffer is copied so it can be safely freed outside the context of the 
     * instance of game_of_life. Throws an invalid_argument if the buffer is 
     * null or the dimensions are out of range.
     */
    static game_of_life* create_from_buffer(char* buf, int width, int height);
    
    /**
     * Loads a board from a plain pbm file. Throws runtime_error if the file 
     * cannot be opened or is not a pbm file, overflow error if the dimensions
     * overflow the long datatype, or out_of_range if the dimensions are out
     * of the min and max ranges specified above.
     */
    static game_of_life* create_from_file(const std::string& filename);
    
    /**
     * Generates a random board of the specified width, height, and population
     * percentage. Throws an invalid_argument if any of the width, height, or 
     * percent are out of range.
     */
    static game_of_life* create_random(int width, int height, int percent);
};

/**
 * The following templates compare values (board, width, height) with values
 * specifically for game of life (width and height bounds) defined here, 
 * therefore, they must be defined here and not in util.hpp.
 */ 
template <class E>
void throw_board_null(char* board) 
{
    throw_null<E>(board, "board cannot be null");
}

template <class E>
void throw_width_out_of_range(int width) 
{
    std::string msg = "width out of range of " + std::to_string(min_width); 
    msg += " and " + std::to_string(max_width);
    throw_false<E>(in_range(width, min_width, max_width), msg);
}

template <class E>
void throw_height_out_of_range(int height) 
{
    std::string msg = "height out of range of " + std::to_string(min_height);
    msg += " and " + std::to_string(max_height);
    throw_false<E>(in_range(height, min_height, max_height), msg);
}

#endif
