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
    int y_start = get_global_id(1) * get_global_size(0);                       \
    int stride = get_global_size(0) * get_global_size(1);                      \
                                                                               \
    for (int y = y_start; y < height; y += stride) {                           \
        int y_north = y ? y - 1 : height - 1;                                  \
        int y_south = (y + 1) == height ? 0 : y + 1;                           \
                                                                               \  
        char##WIDTH n_cells = vload##WIDTH(y_north, grid);                     \
        char##WIDTH ne_cells = shuffle(n_cells, (uchar##WIDTH)e_mask);         \
        char##WIDTH nw_cells = shuffle(n_cells, (uchar##WIDTH)w_mask);         \
                                                                               \
        char##WIDTH cells = vload##WIDTH(y, grid);                             \
        char##WIDTH e_cells = shuffle(cells, (uchar##WIDTH)e_mask);            \
        char##WIDTH w_cells = shuffle(cells, (uchar##WIDTH)w_mask);            \
                                                                               \
        char##WIDTH s_cells = vload##WIDTH(y_south, grid);                     \
        char##WIDTH se_cells = shuffle(s_cells, (uchar##WIDTH)e_mask);         \
        char##WIDTH sw_cells = shuffle(s_cells, (uchar##WIDTH)w_mask);         \
                                                                               \
        char##WIDTH neighbors = n_cells + ne_cells + nw_cells + e_cells +      \
                                w_cells + s_cells + se_cells + sw_cells;       \
        char##WIDTH alive = ((neighbors == (char##WIDTH)(3)) |                 \
            ((neighbors == (char##WIDTH)(2)) & cells)) & (char##WIDTH)(1);     \
        vstore##WIDTH(alive, y, buf);                                          \
    }                                                                          \
}

template_width_pow2_le16(2, (1, 0), (1, 0))

template_width_pow2_le16(4, (3, 0, 1, 2), (1, 2, 3, 0))

template_width_pow2_le16(8, (7, 0, 1, 2, 3, 4, 5, 6), (1, 2, 3, 4, 5, 6, 7, 0))

template_width_pow2_le16(16, (15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14), 
    (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0))

/* Width 4, processes 4 rows in parallel per work item. */
kernel void kernel_width_4_cells_mul16(global char* grid, global char* buf, int width, int height)
{
    int y_start = get_global_id(1) * get_global_size(0) * 4;
    int stride = get_global_size(0) * get_global_size(1) * 4;

    for (int y = y_start; y < height; y += stride) {
        int y_north = y ? y - 1 : height - 1;
        int y_south = (y + 4) == height ? 0 : y + 4;

        char4 row_0 = vload4(y, grid);
        char4 row_1 = vload4(y + 1, grid);
        char4 row_2 = vload4(y + 2, grid);
        char4 row_3 = vload4(y + 3, grid);

        char16 neighbors;
        char16 n_cells = (char16)(vload4(y_north, grid), row_0, row_1, row_2);
        char16 ne_cells = shuffle(n_cells, (uchar16)(1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12));
        char16 nw_cells = shuffle(n_cells, (uchar16)(3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14));
        neighbors = n_cells + ne_cells + nw_cells;

        char16 cells = (char16)(row_0, row_1, row_2, row_3);
        char16 e_cells = shuffle(cells, (uchar16)(1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12));
        char16 w_cells = shuffle(cells, (uchar16)(3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14));
        neighbors += e_cells + w_cells;

        char16 s_cells = (char16)(row_1, row_2, row_3, vload4(y_south, grid));
        char16 se_cells = shuffle(s_cells, (uchar16)(1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12));
        char16 sw_cells = shuffle(s_cells, (uchar16)(3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14));
        neighbors += s_cells + se_cells + sw_cells;

        char16 alive = ((neighbors == (char16)(3)) | ((neighbors == (char16)(2)) & cells)) & (char16)(1);
        vstore16(alive, y / 4, buf);
    }
}

/* Width 8, processes 2 rows in parallel per work item. */
kernel void kernel_width_8_cells_mul16(global char* grid, global char* buf, int width, int height)
{
    int y_start = get_global_id(1) * get_global_size(0) * 2;
    int stride = get_global_size(0) * get_global_size(1) * 2;

    for (int y = y_start; y < height; y += stride) {
        int y_north = y ? y - 1 : height - 1;
        int y_south = (y + 2) == height ? 0 : y + 2;

        char8 row_0 = vload8(y, grid);
        char8 row_1 = vload8(y + 1, grid);

        char16 neighbors;
        char16 n_cells = (char16)(vload8(y_north, grid), row_0);
        char16 ne_cells = shuffle(n_cells, (uchar16)(1, 2, 3, 4, 5, 6, 7, 0, 9, 10, 11, 12, 13, 14, 15, 8));
        char16 nw_cells = shuffle(n_cells, (uchar16)(7, 0, 1, 2, 3, 4, 5, 6, 15, 8, 9, 10, 11, 12, 13, 14));
        neighbors = n_cells + ne_cells + nw_cells;

        char16 cells = (char16)(row_0, row_1);
        char16 e_cells = shuffle(cells, (uchar16)(1, 2, 3, 4, 5, 6, 7, 0, 9, 10, 11, 12, 13, 14, 15, 8));
        char16 w_cells = shuffle(cells, (uchar16)(7, 0, 1, 2, 3, 4, 5, 6, 15, 8, 9, 10, 11, 12, 13, 14));
        neighbors += e_cells + w_cells;

        char16 s_cells = (char16)(row_1, vload8(y_south, grid));
        char16 se_cells = shuffle(s_cells, (uchar16)(1, 2, 3, 4, 5, 6, 7, 0, 9, 10, 11, 12, 13, 14, 15, 8));
        char16 sw_cells = shuffle(s_cells, (uchar16)(7, 0, 1, 2, 3, 4, 5, 6, 15, 8, 9, 10, 11, 12, 13, 14));
        neighbors += s_cells + se_cells + sw_cells;

        char16 alive = ((neighbors == (char16)(3)) | ((neighbors == (char16)(2)) & cells)) & (char16)(1);
        vstore16(alive, y / 2, buf);
    }
}

/*******************************************************************************
 * Kernel for widths greater than 16
 ******************************************************************************/

// Shifts in a value into the vector at the first index 
#define shift_in_first_16(val, vec) (shuffle2((char16)(val), (vec), \
(uchar16)(15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30)))

// Shifts in a value into the vector at the last index 
#define shift_in_last_16(val, vec) (shuffle2((vec), (char16)(val), \
(uchar16)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)))

/* Width greater than 16 and power of 2. */
kernel void kernel_width_gt16_pow2(global char* grid, global char* buf, int width, int height)
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
