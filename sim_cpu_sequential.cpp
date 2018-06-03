/**
 * sim_cpu_sequential.cpp
 * 
 * Optimized sequential CPU implementation of Conway's Game of Life.
 * 
 * Formerly called game_of_life_cpu_sequential.cpp.
 * 
 * Author: Carl Marquez
 * Created on: May 5, 2018
 */
#include <cstring>
#include "sim.hpp"
#include "util.hpp"

/* Simulates the cells in row y for one generation. The input is grid and the 
 * output is buf. */
static inline void cpu_seq_row(char* grid, char* buf, int width, int height, 
    int y, int ynorth, int ysouth)
{
    int irow = y * width;
    int inorth = ynorth * width;
    int isouth = ysouth * width;

    /* First cell of every row is a special case because the west neighbors wrap
    around. */ 
    int x = 0;
    int idx = irow;
    int xwest = width - 1;
    int xeast = 1;
    char cell = grid[inorth + xwest] + grid[inorth] + grid[inorth + xeast] + 
                grid[irow + xwest] + grid[irow + xeast] + 
                grid[isouth + xwest] + grid[isouth] + grid[isouth + xeast];
    cell = (cell == 3) | ((cell == 2) & grid[irow]);
    buf[irow] = cell;
    
    /* Middle cells are handled as expected, west and east neighbors are one 
    less and one more than the row index, respectively. */
    for (x = 1; x < width - 1; ++x) {
        idx = irow + x;
        xwest = x - 1;
        xeast = x + 1;
        cell = grid[inorth + xwest] + grid[inorth + x] + 
               grid[inorth + xeast] + grid[irow + xwest] + 
               grid[irow + xeast] + grid[isouth + xwest] + 
               grid[isouth + x] + grid[isouth + xeast];
        cell = (cell == 3) | ((cell == 2) & grid[idx]);
        buf[idx] = cell;
    }

    /* Last cell of every row is a special case because the east neighbors wrap
    around. */
    x = width - 1;
    idx = irow + x;
    xwest = width - 2;
    xeast = 0;
    cell = grid[inorth + xwest] + grid[inorth + x] + grid[inorth + xeast] + 
           grid[irow + xwest] + grid[irow + xeast] + grid[isouth + xwest] +
           grid[isouth + x] + grid[isouth + xeast];
    cell = (cell == 3) | ((cell == 2) & grid[idx]);
    buf[idx] = cell;
}

void sim_cpu_sequential(char* grid, int width, int height, int gens)
{
    int size = width * height;
    char* buf = new char[size];
    
    /* To optimize performance, the first and last rows are handled separately
    from the rest of the rows because some of their neighbors wrap around. This
    prevents having to do a conditional check every iteration, which has 
    significant overhead. */
    for (int i = 0; i < gens; ++i) {
        cpu_seq_row(grid, buf, width, height, 0, height - 1, 1); // First row

        for (int y = 1; y < height - 1; ++y) // Middle rows
            cpu_seq_row(grid, buf, width, height, y, y - 1, y + 1);

        // Last row
        cpu_seq_row(grid, buf, width, height, height - 1, height - 2, 0);
        swap_ptr(char*, grid, buf);
    }

    /* Always want both grid and buf to be pointing at their original addresses
    after the simulation. If the number of generations is an odd number, the
    result would be in buf and buf and grid would be flipped. */
    if (gens & 1) { 
        swap_ptr(char*, buf, grid);
        memcpy(grid, buf, size);
    }
    delete[] buf; 
}
