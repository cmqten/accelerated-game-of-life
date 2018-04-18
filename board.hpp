/**
 * Functions for loading and saving a board to a pbm file. The pbm file must be
 * a plain pbm file which follow the format below:
 * 
 * P1
 * <width> <height>
 * <board>
 * 
 * - Every line must have no whitespaces around it and the last non-whitespace 
 *   character must be immediately followed by a newline character
 * - Width and height must be separated by a single space
 * - The board must be a string of '0' and '1' characters with no whitespaces in
 *   between
 * 
 * This specification is a subset of the plain pbm format. For simplicity of 
 * checking, a plain pbm file that follows the netpbm specifications but 
 * violates above may not be loaded. For more information about the netpbm
 * specification, visit http://netpbm.sourceforge.net/doc/pbm.html
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
 * false if at least one condition is not met. Helper function because this 
 * check is done in a lot of functions.
 */
inline bool board_width_height_okay(char* board, int width, int height)
{
    return board && in_range(width, MIN_WIDTH, MAX_WIDTH) && 
        in_range(height, MIN_HEIGHT, MAX_HEIGHT);
}

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
 *      An array of 0 and 1 integers
 * 
 * Throws:
 *      runtime_error : invalid file/file format
 *      overflow_error : dimensions overflow the int datatype
 *      range_error : dimensions are out of the range specified above
 */
char* load_board(int* width, int* height, std::string filename);

/**
 * Saves board to file.
 * 
 * Arguments:
 *      board : an array of 0 and 1 integers
 *      width : width of the board in pixels
 *      height : height of the board in pixels
 *      filename : filename to which the board will be saved
 * 
 * Returns:
 *      None
 * 
 * Throws:
 *      logic_error : null board or dimensions out of range
 *      runtime_error : invalid file/unwritable file
 */
void save_board(char* board, int width, int height, std::string filename); 

#endif