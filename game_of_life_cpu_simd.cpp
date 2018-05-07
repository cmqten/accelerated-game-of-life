#include <cstdlib>
#include <cstring>
#include <functional>
#include <x86intrin.h>
#include "simulators.hpp"
#include "util.hpp"

/** ==========================================================================
 * Implementation which uses vectors of 16 characters. Uses SSE2 and SSSE3 
 * extensions.
 * ========================================================================== */
static inline __m128i alive_vec16(__m128i count, __m128i state)
{
    return _mm_and_si128(
        _mm_or_si128(
            _mm_cmpeq_epi8(count, _mm_set1_epi8(3)), 
            _mm_and_si128(state, _mm_cmpeq_epi8(count, _mm_set1_epi8(2)))
        ), 
        _mm_set1_epi8(1)
    );
}

static void game_of_life_cpu_vec16(char* board, int width, int height, int gens)
{
    throw_false<std::invalid_argument>(width >= 16, "width of the board must be"
        " at least 16");

    int size = width * height;
    char* buf = new char[size];

    if (width == 16) {
        for (int i = 0; i < gens; ++i) {
            // Traverse every cell in row major order for every generation
            for (int y = 0; y < height; ++y) {
                int irow = y * width;
                int inorth = (y ? y - 1 : height - 1) * width;
                int isouth = ((y == (height - 1)) ? 0 : (y + 1)) * width;

                __m128i cells;
                __m128i n_cells = _mm_load_si128((__m128i*)(board + inorth));
                cells = n_cells;
                __m128i ne_cells = _mm_alignr_epi8(n_cells, n_cells, 1);
                cells = _mm_add_epi8(cells, ne_cells);
                __m128i nw_cells = _mm_alignr_epi8(n_cells, n_cells, 15);
                cells = _mm_add_epi8(cells, nw_cells);

                __m128i r_cells = _mm_load_si128((__m128i*)(board + irow));
                __m128i e_cells = _mm_alignr_epi8(r_cells, r_cells, 1);
                cells = _mm_add_epi8(cells, e_cells);
                __m128i w_cells = _mm_alignr_epi8(r_cells, r_cells, 15);
                cells = _mm_add_epi8(cells, w_cells);

                __m128i s_cells = _mm_load_si128((__m128i*)(board + isouth));
                cells = _mm_add_epi8(cells, s_cells);
                __m128i se_cells = _mm_alignr_epi8(s_cells, s_cells, 1);
                cells = _mm_add_epi8(cells, se_cells);
                __m128i sw_cells = _mm_alignr_epi8(s_cells, s_cells, 15);
                cells = _mm_add_epi8(cells, sw_cells);

                cells = alive_vec16(cells, r_cells);
                _mm_store_si128((__m128i*)(buf + irow), cells);
            }
            swap_ptr(char*, board, buf);
        }
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

/**
 * Single thread implementation which uses MMX, SSE, and SSE2 extensions.
 */ 
void game_of_life_cpu_simd(char* board, int width, int height, int gens)
{
    if (width == 16) {
        game_of_life_cpu_vec16(board, width, height, gens);
    }
}