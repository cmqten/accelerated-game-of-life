/**
 * Generates a random game of life board according to the specified population
 * percentage and size and saves it as a pbm file.
 */
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>

/**
 * Print an error message and exit with a non-zero status code.
 */
static inline void exit_error(std::string msg) 
{
    std::cerr << msg << std::endl;
    exit(1);
}

/**
 * Saves board to file.
 */
void save_board(char* board, int width, int height, const std::string& filename) 
{
    int size = width * height;
    if (height != size / width) {
        std::cerr << "Multiplication overflow" << std::endl;
        return;
    }

    std::ofstream file;
    file.open(filename);
    file << "P1" << std::endl;
    file << width << " " << height << std::endl;
    file.write(board, size);
    file.close();
}

/**
 * Generate a random board of the specified size and percent. Returns null if 
 * an error occurs and prints the error to stderr.
 */
char* random_board(int width, int height, int percent) 
{
    int size = width * height;
    if (height != size / width) {
        std::cerr << "Multiplication overflow" << std::endl;
        return nullptr;
    }

    char* board = (char*)calloc(sizeof(char), size);
    if (board == nullptr) {
        std::cerr << "Out of memory" << std::endl;
        return nullptr;
    }

    srand(time(nullptr));
    for (int i = 0; i < size; ++i) {
        board[i] = ((rand() % 100) < percent) + '0'; 
    }

    return board;
}

int main(int argc, char** argv) 
{
    if (argc != 5) exit_error("Usage: generate WIDTH HEIGHT PERCENT FILENAME");

    int width = strtol(argv[1], NULL, 10);
    if (errno) exit_error("Width decimal literal overflow");
    if (!width) exit_error("Width must be a non-zero decimal literal");

    int height = strtol(argv[2], NULL, 10);
    if (errno) exit_error("Height decimal literal overflow");
    if (!height) exit_error("Height must be a non-zero decimal literal");

    int percent = strtol(argv[3], NULL, 10);
    if (errno) exit_error("Percent decimal literal overflow");
    if (percent < 1 || percent > 100) 
        exit_error("Percent must be between 1 and 100 inclusive");

    std::string filename(argv[4]);

    char* board = random_board(width, height, percent);
    if (board == nullptr) return 1;
    save_board(board, width, height, filename);
    free(board);

    return 0;
}
