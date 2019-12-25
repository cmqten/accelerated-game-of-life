/**
 * life_cpu_seq.cpp
 * 
 * Optimized sequential CPU implementation of Conway's Game of Life.
 * 
 * Author: Carl Marquez
 * Created on: May 5, 2018
 */
#include <cstring>

#include <game_of_life.hpp>
#include <util.hpp>

/* Processes cells in a row. */
static inline void cpu_seq_row(char* grid, char* buf, int width, int y, int ynorth, int ysouth)
{
    int i_row = y * width;
    int i_north = ynorth * width;
    int i_south = ysouth * width;

    // First cell is a special case because the west neighbors wrap around. 
    int x = 0;
    int idx = i_row;
    int x_west = width - 1;
    int x_east = 1;
    char cell = grid[i_north + x_west] + grid[i_north] + 
                grid[i_north + x_east] + grid[i_row + x_west] + 
                grid[i_row + x_east] + grid[i_south + x_west] + 
                grid[i_south] + grid[i_south + x_east];
    cell = (cell == 3) | ((cell == 2) & grid[i_row]);
    buf[i_row] = cell;
    
    // Middle cells
    for (x = 1; x < width - 1; x++) {
        idx = i_row + x;
        x_west = x - 1;
        x_east = x + 1;
        cell = grid[i_north + x_west] + grid[i_north + x] + 
               grid[i_north + x_east] + grid[i_row + x_west] + 
               grid[i_row + x_east] + grid[i_south + x_west] + 
               grid[i_south + x] + grid[i_south + x_east];
        cell = (cell == 3) | ((cell == 2) & grid[idx]);
        buf[idx] = cell;
    }

    // Last cell is a special case because the east neighbors wrap around.
    x = width - 1;
    idx = i_row + x;
    x_west = width - 2;
    x_east = 0;
    cell = grid[i_north + x_west] + grid[i_north + x] + grid[i_north + x_east] + 
           grid[i_row + x_west] + grid[i_row + x_east] + 
           grid[i_south + x_west] + grid[i_south + x] + grid[i_south + x_east];
    cell = (cell == 3) | ((cell == 2) & grid[idx]);
    buf[idx] = cell;
}

void cpu_seq(char* grid, int width, int height, int gens)
{
    int size = width * height;
    char* buf = new char[size];
    
    for (int i = 0; i < gens; i++) {
        // First and last rows are outside of the loop to not have to check
        // for north and south neighbor bounds.
        cpu_seq_row(grid, buf, width, 0, height - 1, 1);
        for (int y = 1; y < height - 1; y++) {
            cpu_seq_row(grid, buf, width, y, y - 1, y + 1);
        }
        cpu_seq_row(grid, buf, width, height - 1, height - 2, 0);
        swap_ptr((void**)&grid, (void**)&buf);
    }

    // If number of generations is odd, the result is in buf, so swap with grid. 
    if (gens % 2) { 
        swap_ptr((void**)&grid, (void**)&buf);
        memcpy(grid, buf, size);
    }
    delete[] buf; 
}
