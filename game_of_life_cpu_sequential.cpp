#include <cstdlib>
#include <cstring>
#include "simulators.hpp"
#include "util.hpp"

/**
 * Unoptimized CPU sequential implementation
 */ 
void game_of_life_cpu_sequential(char* board, int width, int height, int gens)
{
    int size = width * height;
    char* buf = new char[size];

    for (int i = 0; i < gens; ++i) {
        // Traverse every cell in row major order for every generation
        for (int y = 0; y < height; ++y) {
            int irow = y * width;
            int inorth = (y ? y - 1 : height - 1) * width;
            int isouth = ((y == (height - 1)) ? 0 : (y + 1)) * width;
            
            for (int x = 0; x < width; ++x) {
                int idx = irow + x;
                int iwest = x ? (x - 1) : (width - 1);
                int ieast = (x == (width - 1)) ? 0 : x + 1;

                // Calculate neigbors
                char cell = board[inorth + iwest] + board[inorth + x] + 
                            board[inorth + ieast] + board[irow + iwest] + 
                            board[irow + ieast] + board[isouth + iwest] +
                            board[isouth + x] + board[isouth + ieast];
                
                // Calculate if alive or dead
                cell = (cell == 3) | ((cell == 2) & board[idx]);
                buf[idx] = cell;
            }
        }
        swap_ptr(char*, board, buf);
    }

    /**
     * Always want both board and buf to be pointing at their original addresses
     * after the simulation. If the number of generations is an odd number, the
     * result would be in buf and buf and board would be flipped.
     */
    if (gens & 1) { 
        swap_ptr(char*, buf, board);
        memcpy(board, buf, size);
    }
    delete[] buf; 
}
