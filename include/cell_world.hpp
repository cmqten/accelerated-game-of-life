/**
 * cell_world.hpp
 * 
 * Conway's Game of Life implementation using C++ using various techniques to
 * optimize for performance. Game of Life is a life simulation which simulates
 * a grid of cells. In every generation, each cell can be alive or dead, and the
 * state of each cell in the next generation is determined by the following 
 * rules:
 * 
 * 1. A living cell with less than two neighbors dies due to loneliness.
 * 2. A living cell with more than three neighbors dies due to overpopulation.
 * 3. A living cell with two or three neighbors survives.
 * 4. A dead cell with three neighbors becomes alive due to reproduction.
 * 
 * Implementation details:
 * Grid:
 * A grid is represented by a char array. The array may be generated randomly or
 * loaded from a plain pbm file. The pbm file must have the following format:
 * 
 * ---------------
 * Line | Contents
 * ---------------
 *   1  | P1
 *   2  | <width> <height>
 *   3  | <grid>
 * 
 * The first line must contain the string "P1" at the first index immediately 
 * followed by a newline character. 
 * 
 * The second line must have the width as a decimal literal at the first index, 
 * followed by a single space then the height as a decimal literal, immediately 
 * followed by a newline character. 
 * 
 * The last line must contain a string of '0' and '1' with no spaces in between 
 * them. 
 * 
 * This format is a subset of the plain pbm specifications, so some pbm files 
 * that conform to the specifications may not be loaded for simplicity of 
 * checking and loading. For more information about the netpbm specifications, 
 * visit: http://netpbm.sourceforge.net/doc/pbm.html
 * 
 * Simulator:
 * All functions that simulate Game of Life must have the following signature:
 * 
 * void foo(char* grid, int width, int height, int gens)
 * 
 * The simulation result must be in the same buffer as the one in the arguments.
 * 
 * Formerly called game_of_life.hpp.
 * 
 * Author: Carl Marquez
 * Created on: April 21, 2018
 */
#ifndef __CELL_WORLD_HPP__
#define __CELL_WORLD_HPP__

#include <functional>
#include <string>
#include "util.hpp"

/* Minimum dimension is 3 so that every cell has at least 8 neighbors and 
maximum dimension is 16384 to prevent the system from running out of memory. */
const int min_width = 3;
const int max_width = 16384;
const int min_height = 3;
const int max_height = 16384;

class cell_world 
{
private:
    explicit cell_world(char* grid, int width, int height, 
        std::function<void(char*, int, int, int)> lifesim);
    
    char* grid;

public:
    const int width;
    const int height;
    const int size;
    std::function<void(char*, int, int, int)> lifesim;

    ~cell_world();

    bool operator==(const cell_world& other) const;

    bool operator!=(const cell_world& other) const;
    
    /* Returns copy of grid. Free using delete[]. */
    char* get_grid() const; 

    /* Saves the grid as the specified filename. Throws a runtime_error if the
     * file cannot be opened for writing. */
    void save_grid(const std::string& filename) const;

    /* Simulates the game of life grid for the specified number of generations
     * and returns the simulation time in seconds. Does nothing and returns -1.0
     * if simulate_func is set to nullptr. */
    double simulate(int gens);

    /* Creates a grid from a pre-existing buffer of 0 and 1 integers. The 
     * buffer is copied so it can be safely freed outside the context of the 
     * instance of cell_world. Throws an invalid_argument if the buffer is 
     * null or the dimensions are out of range. */
    static cell_world create_from_buffer(char* buf, int width, int height,
        std::function<void(char*, int, int, int)> lifesim = nullptr);
    
    /* Creates a copy of an existing cell_world instance. */
    static cell_world create_from_existing(const cell_world& other);
    
    /* Loads a grid from a plain pbm file. Throws runtime_error if the file 
     * cannot be opened or is not a pbm file, overflow error if the dimensions
     * overflow the long datatype, or out_of_range if the dimensions are out
     * of the min and max ranges specified above. */
    static cell_world create_from_file(const std::string& filename,
        std::function<void(char*, int, int, int)> lifesim = nullptr);
    
    /* Generates a random grid of the specified width, height, and population
     * percentage. Throws an invalid_argument if any of the width, height, or 
     * percent are out of range. */
    static cell_world create_random(int width, int height, int percent,
        std::function<void(char*, int, int, int)> lifesim = nullptr);
};

/* The following templates compare values (grid, width, height) with values
 * specifically for game of life (width and height bounds) defined here, 
 * therefore, they must be defined here and not in util.hpp. */ 
template <class E>
void throw_grid_null(char* grid) 
{
    throw_null<E>(grid, "grid cannot be null");
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
