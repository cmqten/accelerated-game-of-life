/**
 * life_ocl_kernel.cl
 *
 * Simulates Conway's Game of Life on a GPU using OpenCL. Handles the case 
 * where each row is strictly the same size as the vector length.
 * 
 * Author: Carl Marquez
 * Created on: June 16, 2018
 */
#if defined VECLEN

#if VECLEN == 2
#define T char2
#define vloadn vload2
#define vstoren vstore2
#define w_mask (uchar2)(1, 0)
#define e_mask (uchar2)(1, 0)

#elif VECLEN == 4
#define T char4
#define vloadn vload4
#define vstoren vstore4
#define w_mask (uchar4)(3, 0, 1, 2)
#define e_mask (uchar4)(1, 2, 3, 0)

#elif VECLEN == 8
#define T char8
#define vloadn vload8
#define vstoren vstore8
#define w_mask (uchar8)(7, 0, 1, 2, 3, 4, 5, 6)
#define e_mask (uchar8)(1, 2, 3, 4, 5, 6, 7, 0)

#elif VECLEN == 16
#define T char16
#define vloadn vload16
#define vstoren vstore16
#define w_mask (uchar16)(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)
#define e_mask (uchar16)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0)

#else 
#undef VECLEN
#define VECLEN 8
#define T char8
#define vloadn vload8
#define vstoren vstore8
#define w_mask (uchar8)(7, 0, 1, 2, 3, 4, 5, 6)
#define e_mask (uchar8)(1, 2, 3, 4, 5, 6, 7, 0)

#endif

#else
#define VECLEN 8
#define T char8
#define vloadn vload8
#define vstoren vstore8
#define w_mask (uchar8)(7, 0, 1, 2, 3, 4, 5, 6)
#define e_mask (uchar8)(1, 2, 3, 4, 5, 6, 7, 0)

#endif

inline T alive(T count, T state)
{
    return ((count == (T)(3)) | ((count == (T)(2)) & state)) & (T)(1);
}

kernel void life_kernel(global char* grid, global char* buf, int width,
    int height)
{
    int i_start = (get_global_id(1) * get_global_size(0) + get_global_id(0)) * 
                  VECLEN;
    int stride = get_global_size(0) * get_global_size(1) * VECLEN;
    int size = width * height;

    for (int i = i_start; i < size; i += stride) {
        int i_north = i ? i - VECLEN : size - VECLEN;
        int i_south = i == (size - VECLEN) ? 0 : i + VECLEN;

        T n_cells = vloadn(0, grid + i_north);
        T ne_cells = shuffle(n_cells, e_mask);
        T nw_cells = shuffle(n_cells, w_mask);
        T cells = vloadn(0, grid + i);
        T e_cells = shuffle(cells, e_mask);
        T w_cells = shuffle(cells, w_mask);
        T s_cells = vloadn(0, grid + i_south);
        T se_cells = shuffle(s_cells, e_mask);
        T sw_cells = shuffle(s_cells, w_mask);

        vstoren(alive(
            n_cells + ne_cells + nw_cells + e_cells + w_cells + s_cells + 
            se_cells + sw_cells, cells 
        ), 0, buf + i);
    }
}