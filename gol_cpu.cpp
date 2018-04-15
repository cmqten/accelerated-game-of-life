/**
 * Game of Life CPU implementations
 */
#include <cstdint>
#include <cstring>
#include <iostream>
#include "board.hpp"
#include "gol.hpp"
#include "util.h"

#define swap_boards(a, b) \
a = (char*)((uintptr_t)a ^ (uintptr_t)b); \
b = (char*)((uintptr_t)a ^ (uintptr_t)b); \
a = (char*)((uintptr_t)a ^ (uintptr_t)b);

#define board_elem(_board, _x, _y, _width) ((_board)[(_y) * (_width) + (_x)]);

inline int process_cell(char* board, int width, int height, int x, int y)
{
    char neighbors = 0;
    char cell = board_elem(board, x, y, width);
    int inorth = mod((y - 1), height);
    int isouth = mod((y + 1), height);
    int iwest = mod((x - 1), width);
    int ieast = mod((x + 1), width);

    // Neighbor count
    neighbors = board_elem(board, iwest, inorth, width) + 
                board_elem(board, x, inorth, width) + 
                board_elem(board, ieast, inorth, width) + 
                board_elem(board, iwest, y, width) + 
                board_elem(board, ieast, y, width) + 
                board_elem(board, iwest, isouth, width) +  
                board_elem(board, x, isouth, width) + 
                board_elem(board, ieast, isouth, width);
    
    // Alive or dead
    if (cell) return neighbors == 2 || neighbors == 3;
    return neighbors == 3;
}

char* gol_seq(char* board, int width, int height, int gens)
{
    if (!board_width_height_okay(board, width, height)) return nullptr;
    int size = width * height;
    char* buf1 = new char[size];
    char* buf2 = new char[size];
    memcpy(buf1, board, size);

    for (int i = 0; i < gens; ++i) {
        for (int y = 0; y < height; ++y) {
            int row_idx = y * width;
            for (int x = 0; x < width; ++x) {
                buf2[row_idx + x] = process_cell(buf1, width, height, x, y);
            }
        }
        swap_boards(buf1, buf2);
    }

    delete[] (gens & 1 ? buf1 : buf2);
    return gens & 1 ? buf2 : buf1;
}
