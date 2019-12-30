/**
 * kernels.cl
 *
 * Simulates Conway's Game of Life on a GPU using OpenCL.
 * 
 * Author: Carl Marquez
 * Created on: June 16, 2018
 */

/*******************************************************************************
 * Kernel for widths of 2, 4, 8, 16
 ******************************************************************************/
 
#define template_width_pow2_le16(WIDTH, w_mask, e_mask)                        \
kernel void kernel_width_##WIDTH(global char* grid, global char* buf, int width, int height) \
{                                                                              \
    int y_start = get_global_id(1) * get_global_size(0) + get_global_id(0);    \
    int stride = get_global_size(0) * get_global_size(1);                      \
                                                                               \
    for (int y = y_start; y < height; y += stride) {                           \
        int y_north = y ? y - 1 : height - 1;                                  \
        int y_south = (y + 1) == height ? 0 : y + 1;                           \
        int i_row = y * WIDTH;                                                 \
        int i_north = y_north * WIDTH;                                         \
        int i_south = y_south * WIDTH;                                         \
                                                                               \  
        char##WIDTH n_cells = vload##WIDTH(0, grid + i_north);                 \
        char##WIDTH ne_cells = shuffle(n_cells, (uchar##WIDTH)e_mask);         \
        char##WIDTH nw_cells = shuffle(n_cells, (uchar##WIDTH)w_mask);         \
                                                                               \
        char##WIDTH cells = vload##WIDTH(0, grid + i_row);                     \
        char##WIDTH e_cells = shuffle(cells, (uchar##WIDTH)e_mask);            \
        char##WIDTH w_cells = shuffle(cells, (uchar##WIDTH)w_mask);            \
                                                                               \
        char##WIDTH s_cells = vload##WIDTH(0, grid + i_south);                 \
        char##WIDTH se_cells = shuffle(s_cells, (uchar##WIDTH)e_mask);         \
        char##WIDTH sw_cells = shuffle(s_cells, (uchar##WIDTH)w_mask);         \
                                                                               \
        char##WIDTH neighbors = n_cells + ne_cells + nw_cells + e_cells +      \
                                w_cells + s_cells + se_cells + sw_cells;       \
        char##WIDTH alive = ((neighbors == (char##WIDTH)(3)) |                 \
            ((neighbors == (char##WIDTH)(2)) & cells)) & (char##WIDTH)(1);     \
        vstore##WIDTH(alive, 0, buf + i_row);                                  \
    }                                                                          \
}

template_width_pow2_le16(2, (1, 0), (1, 0))

template_width_pow2_le16(4, (3, 0, 1, 2), (1, 2, 3, 0))

template_width_pow2_le16(8, (7, 0, 1, 2, 3, 4, 5, 6), (1, 2, 3, 4, 5, 6, 7, 0))

template_width_pow2_le16(16, (15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14), 
    (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0))

/*******************************************************************************
 * Kernel for widths greater than 16
 ******************************************************************************/

// Shifts in a value into the vector at the first index 
#define shift_in_first_16(val, vec) (shuffle2((char16)(val), (vec), \
(uchar16)(15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30)))

// Shifts in a value into the vector at the last index 
#define shift_in_last_16(val, vec) (shuffle2((vec), (char16)(val), \
(uchar16)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)))

kernel void kernel_width_gt16(global char* grid, global char* buf, int width, int height)
{
    int x_start = get_global_id(0) * 16;
    int y_start = get_global_id(1);
    int global_width = get_global_size(0);
    int global_height = get_global_size(1);
    int stride = global_width * 16;

    for (int y = y_start; y < height; y += global_height) {
        int y_north = y ? y - 1 : height - 1;
        int y_south = (y + 1) == height ? 0 : y + 1;
        int i_row = y * width;
        int i_north = y_north * width;
        int i_south = y_south * width;

        global char* p_north = grid + i_north;
        global char* p_row = grid + i_row;
        global char* p_south = grid + i_south;

        for (int x = x_start; x < width; x += stride) {
            if (width - x < 16) x = width - 16;
            int x_west = x ? x - 1 : width - 1;
            int x_east = (x + 16) == width ? 0 : x + 16;

            char16 n_cells = vload16(0, p_north + x);
            char16 nw_cells = shift_in_first_16(p_north[x_west], n_cells);
            char16 ne_cells = shift_in_last_16(p_north[x_east], n_cells);
            
            char16 cells = vload16(0, p_row + x);
            char16 w_cells = shift_in_first_16(p_row[x_west], cells);
            char16 e_cells = shift_in_last_16(p_row[x_east], cells);

            char16 s_cells = vload16(0, p_south + x);
            char16 sw_cells = shift_in_first_16(p_south[x_west], s_cells);
            char16 se_cells = shift_in_last_16(p_south[x_east], s_cells);

            char16 neighbors = n_cells + ne_cells + nw_cells + e_cells + w_cells + s_cells + se_cells + sw_cells;
            char16 alive = (((neighbors == (char16)(3)) | ((neighbors == (char16)(2)) & cells)) & (char16)(1));
            vstore16(alive, 0, buf + i_row + x);
        }
    }
}
