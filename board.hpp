/**
 * Functions for loading and saving a board to a pbm file.
 */
#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include <string>

#define MIN_WIDTH 1
#define MAX_WIDTH 32768
#define MIN_HEIGHT 1
#define MAX_HEIGHT 32768

/**
 * Converts an array of '0' and '1' char to an array of 0 and 1 int. The
 * precondition is that it is an array of only '0' and '1'
 */
bool ascii_to_int(char* board, int width, int height);

/**
 * Converts an array of 0 and 1 int to an array of '0' and '1' char. The
 * precondition is that it is an array of only 0 and 1.
 */
bool int_to_ascii(char* board, int width, int height);

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
 *      None
 */
void save_board(char* board, int width, int height, std::string filename); 

/**
 * Loads board from file.
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

#endif