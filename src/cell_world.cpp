/**
 * cell_world.cpp
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
 * Formerly called game_of_life.cpp
 * 
 * Author: Carl Marquez
 * Created on: April 21, 2018
 */
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cell_world.hpp>

/* A copy of buf is made to ensure that the grid is allocated correctly (using
 * new) since grid is deallocated using delete[] in the destructor of 
 * cell_world. */
cell_world cell_world::create_from_buffer(char* buf, int width, int height,
    std::function<void(char*, int, int, int)> simulator)
{
    throw_grid_null<std::invalid_argument>(buf);
    throw_width_out_of_range<std::invalid_argument>(width);
    throw_height_out_of_range<std::invalid_argument>(height);

    int size = width * height;
    char* grid = new char[size];
    memcpy(grid, buf, size);
    return cell_world(grid, width, height, simulator);
}

/* Copies an existing instance of cell_world. Use this instead of the copy
 * constructor because the copy constructor does not make a new copy of the 
 * grid. */
cell_world cell_world::create_from_existing(const cell_world& other) 
{
    return cell_world::create_from_buffer(other.grid, other.width, other.height,
        other.simulator);
}

/* The file must be a plain pbm file with the specified format in the header
 * file cell_world.hpp. */
cell_world cell_world::create_from_file(const std::string& filename,
    std::function<void(char*, int, int, int)> simulator) 
{
    std::ifstream file;
    file.open(filename);
    throw_true<std::runtime_error>(!file, "file cannot be opened for reading");

    // Checks for the magic number "P1"
    std::string line;
    std::getline(file, line);
    throw_non_zero<std::runtime_error>(line.compare("P1"), "not a pbm file");

    // The second line is the dimensions
    char* remain;
    std::getline(file, line);
    int width = strtol(line.c_str(), &remain, 10);
    throw_non_zero<std::overflow_error>(errno, "width overflow/underflow");
    throw_width_out_of_range<std::out_of_range>(width);

    int height = strtol(remain, nullptr, 10);
    throw_non_zero<std::overflow_error>(errno, "height overflow/underflow");
    throw_height_out_of_range<std::out_of_range>(height);

    // The third line is a string of '0' and '1' that represents the grid
    int size = width * height;
    char* grid = new char[size];
    file.read(grid, size);   
    file.close();
    for (int i = 0; i < size; ++i) grid[i] -= '0'; // ascii to int
    return cell_world(grid, width, height, simulator);
}

cell_world cell_world::create_random(int width, int height, int percent,
    std::function<void(char*, int, int, int)> simulator)
{
    throw_width_out_of_range<std::invalid_argument>(width);
    throw_height_out_of_range<std::invalid_argument>(height);
    throw_false<std::invalid_argument>(in_range(percent, 1, 100), 
        "percent out of range of 1 and 100");

    int size = width * height;
    char* grid = new char[size];
    srand(time(nullptr));
    for (int i = 0; i < size; ++i) grid[i] = ((rand() % 100) < percent);
    return cell_world(grid, width, height, simulator);
}

cell_world::cell_world(char* grid, int width, int height,
    std::function<void(char*, int, int, int)> simulator) : 
    grid(grid), width(width), height(height), size(width * height),
    simulator(simulator)
{ 
}

cell_world::~cell_world() 
{
    delete[] grid;
}

bool cell_world::operator==(const cell_world& other) const
{
    return width == other.width && height == other.height && size == other.size
        && !memcmp(grid, other.grid, size);
}

bool cell_world::operator!=(const cell_world& other) const
{
    return !operator==(other);
}

char* cell_world::get_grid() const
{
    char* grid_copy = new char[size];
    memcpy(grid_copy, grid, size);
    return grid_copy;
}

void cell_world::save_grid(const std::string& filename) const
{
    std::ofstream file;
    file.open(filename);
    throw_true<std::runtime_error>(!file, "file cannot be opened for writing");

    char* buf = new char[size];
    memcpy(buf, grid, size); // Don't want to modify the original grid
    for (int i = 0; i < size; ++i) buf[i] += '0';

    file << "P1\n" << width << " " << height << std::endl;
    file.write(buf, size);
    file.close();
    delete[] buf;
}

double cell_world::simulate(int gens)
{
    if (simulator == nullptr) return -1.0;
    my_timer timer;
    timer.start();
    simulator(grid, width, height, gens);
    return timer.stop();
}
