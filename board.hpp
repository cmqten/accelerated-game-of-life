/**
 * Functions for loading and saving a board to a pbm file.
 */
#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include <iostream>
#include <string>

#define MIN_WIDTH 1
#define MAX_WIDTH 32768
#define MIN_HEIGHT 1
#define MAX_HEIGHT 32768

#define in_range(val, min, max) ((val) >= (min) && (val) <= (max))

/**
 * Returns true if all of the following conditions are satisfied:
 * - board is not null
 * - width and height are both within the range of min and max
 * 
 * false if at least one condition is not met. Prints error message to stderr. 
 * Helper function because this check is done in a lot of functions.
 */
inline bool board_width_height_okay(char* board, int width, int height)
{
    if (!board) {
        std::cerr << "Error: board can't be null" << std::endl;
        return false;
    }
    if (!in_range(width, MIN_WIDTH, MAX_WIDTH)) {
        std::cerr << "Error: width must be between " << MIN_WIDTH << " and ";
        std::cerr << MAX_WIDTH << std::endl;
        return false;
    }
    if (!in_range(height, MIN_HEIGHT, MAX_HEIGHT)) {
        std::cerr << "Error: height must be between " << MIN_HEIGHT << " and ";
        std::cerr << MAX_HEIGHT << std::endl;
        return false;
    }
    return true;
}

/**
 * Converts an array of '0' and '1' char to an array of 0 and 1 int. The
 * precondition is that it is an array of only '0' and '1'. Returns true if the 
 * operation is successful, false otherwise.
 */
bool ascii_to_int(char* board, int width, int height);

/**
 * Converts an array of 0 and 1 int to an array of '0' and '1' char. The
 * precondition is that it is an array of only 0 and 1. Returns true if the 
 * operation is successful, false otherwise.
 */
bool int_to_ascii(char* board, int width, int height);

/**
 * Generates a random board of the specified size and percent. The width and 
 * height must be within the range of the min and max macros defined above, and
 * percent should be between 1 and 100. Returns a generated board if all of the
 * arguments are valid, nullptr otherwise.
 */
char* random_board(int width, int height, int percent);

/**
 * Loads board from file. The char array returned must be freed using delete[].
 * 
 * Arguments:
 *      width : pointer to an int variable that would store the width
 *      height : pointer to an int variable that would store the height
 *      filename : filename of the board to be loaded
 *
 * Returns:
 *      An array of '0' and '1' characters, nullptr if an error occurs
 */
char* load_board(int* width, int* height, std::string filename);

/**
 * Saves board to file.
 * 
 * Arguments:
 *      board : an array of '0' and '1' characters
 *      width : width of the board in pixels
 *      height : height of the board in pixels
 *      filename : filename to which the board will be saved
 * 
 * Returns:
 *      True if the operation is successful, false otherwise
 */
bool save_board(char* board, int width, int height, std::string filename); 

#endif