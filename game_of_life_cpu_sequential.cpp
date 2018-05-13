/**
 * Game of Life CPU Sequential
 * 
 * Optimized sequential CPU implementation of a Game of Life simulator.
 */
#include <cstring>
#include "simulators.hpp"
#include "util.hpp"

/**
 * Simulates the cells in row y for one generation. The input is board and the 
 * output is buf. 
 */
static inline void simulate_row(char* board, char* buf, int width, int height, 
    int y, int ynorth, int ysouth)
{
    int irow = y * width;
    int inorth = ynorth * width;
    int isouth = ysouth * width;

    /* First cell of every row is a special case because the west neighbors wrap
    around. */ 
    int x = 0;
    int idx = irow;
    int iwest = width - 1;
    int ieast = 1;
    char cell = board[inorth + iwest] + board[inorth] + board[inorth + ieast] + 
                board[irow + iwest] + board[irow + ieast] + 
                board[isouth + iwest] + board[isouth] + board[isouth + ieast];
    cell = (cell == 3) | ((cell == 2) & board[irow]);
    buf[irow] = cell;
    
    /* Middle cells are handled as expected, west and east neighbors are one 
    less and one more than the row index, respectively. */
    for (x = 1; x < width - 1; ++x) {
        idx = irow + x;
        iwest = x - 1;
        ieast = x + 1;
        cell = board[inorth + iwest] + board[inorth + x] + 
               board[inorth + ieast] + board[irow + iwest] + 
               board[irow + ieast] + board[isouth + iwest] + 
               board[isouth + x] + board[isouth + ieast];
        cell = (cell == 3) | ((cell == 2) & board[idx]);
        buf[idx] = cell;
    }

    /* Last cell of every row is a special case because the east neighbors wrap
    around. */
    x = width - 1;
    idx = irow + x;
    iwest = width - 2;
    ieast = 0;
    cell = board[inorth + iwest] + board[inorth + x] + board[inorth + ieast] + 
           board[irow + iwest] + board[irow + ieast] + board[isouth + iwest] +
           board[isouth + x] + board[isouth + ieast];
    cell = (cell == 3) | ((cell == 2) & board[idx]);
    buf[idx] = cell;
}

void game_of_life_cpu_sequential(char* board, int width, int height, int gens)
{
    int size = width * height;
    char* buf = new char[size];
    
    for (int i = 0; i < gens; ++i) {
        // First row is a special case because the north neighbors wrap around.
        simulate_row(board, buf, width, height, 0, height - 1, 1);

        // Middle rows
        for (int y = 1; y < height - 1; ++y)
            simulate_row(board, buf, width, height, y, y - 1, y + 1);

        // Last row is a special case because the south neighbors wrap around.
        simulate_row(board, buf, width, height, height - 1, height - 2, 0);
        swap_ptr(char*, board, buf);
    }

    /* Always want both board and buf to be pointing at their original addresses
    after the simulation. If the number of generations is an odd number, the
    result would be in buf and buf and board would be flipped. */
    if (gens & 1) { 
        swap_ptr(char*, buf, board);
        memcpy(board, buf, size);
    }
    delete[] buf; 
}
