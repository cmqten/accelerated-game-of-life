#if defined VECLEN

#if VECLEN == 2
#define T char2
#define vloadn vload2
#define vstoren vstore2
#define w_mask (uchar2)(1, 2)
#define e_mask (uchar2)(1, 2)

#elif VECLEN == 4
#define T char4
#define vloadn vload4
#define vstoren vstore4
#define w_mask (uchar4)(3, 4, 5, 6)
#define e_mask (uchar4)(1, 2, 3, 4)

#elif VECLEN == 8
#define T char8
#define vloadn vload8
#define vstoren vstore8
#define w_mask (uchar8)(7, 8, 9, 10, 11, 12, 13 ,14)
#define e_mask (uchar8)(1, 2, 3, 4, 5, 6, 7, 8)

#elif VECLEN == 16
#define T char16
#define vloadn vload16
#define vstoren vstore16
#define w_mask (uchar16)(15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, \
               28, 29, 30)
#define e_mask (uchar16)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)

#else 
#undef VECLEN
#define VECLEN 8
#define T char8
#define vloadn vload8
#define vstoren vstore8
#define w_mask (uchar8)(7, 8, 9, 10, 11, 12, 13 ,14)
#define e_mask (uchar8)(1, 2, 3, 4, 5, 6, 7, 8)

#endif

#else
#define VECLEN 8
#define T char8
#define vloadn vload8
#define vstoren vstore8
#define w_mask (uchar8)(7, 8, 9, 10, 11, 12, 13 ,14)
#define e_mask (uchar8)(1, 2, 3, 4, 5, 6, 7, 8)

#endif

inline T alive(T count, T state)
{
    return ((count == (T)(3)) | ((count == (T)(2)) & state)) & 
        (T)(1);
}

kernel void life_kernel(global char* grid, global char* buf, int width,
    int height)
{
    int x_start = get_global_id(0) * VECLEN;
    int y_start = get_global_id(1);
    int w_width = get_global_size(0);
    int w_height = get_global_size(1);
    int stride = w_width * VECLEN;

    if (x_start == 0) {
        for (int y = y_start; y < height; y += w_height) {
            int ynorth = y ? y - 1 : height - 1;
            int ysouth = y == (height - 1) ? 0 : y + 1;
            int irow = y * width;
            int inorth = ynorth * width;
            int isouth = ysouth * width;

            global char* rnorth = grid + inorth;
            global char* row = grid + irow;
            global char* rsouth = grid + isouth; 

            // Do the first vector
            T n_cells = vloadn(0, rnorth);
            T ne_cells = shuffle2(n_cells, (T)(*(rnorth + VECLEN)),
                e_mask);
            T nw_cells = shuffle2((T)(*(rnorth + width - 1)), n_cells, 
                w_mask);

            T cells = vloadn(0, row);
            T e_cells = shuffle2(cells, (T)(*(row + VECLEN)),
                e_mask);
            T w_cells = shuffle2((T)(*(row + width - 1)), cells, 
                w_mask);

            T s_cells = vloadn(0, rsouth);
            T se_cells = shuffle2(s_cells, (T)(*(rsouth + VECLEN)),
                e_mask);
            T sw_cells = shuffle2((T)(*(rsouth + width - 1)), s_cells, 
                w_mask);

            vstoren(alive(
                n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                s_cells + se_cells + sw_cells, cells 
            ), 0, buf + irow);

            // Do the middle vectors
            for (int x = stride; x < width - VECLEN; x += stride) {
                int xwest = x - 1;

                n_cells = vloadn(0, rnorth + x);
                ne_cells = shuffle2(n_cells, (T)(*(rnorth + x + VECLEN)),
                    e_mask);
                nw_cells = shuffle2((T)(*(rnorth + xwest)), n_cells, 
                    w_mask);

                cells = vloadn(0, row + x);
                e_cells = shuffle2(cells, (T)(*(row + x + VECLEN)),
                    e_mask);
                w_cells = shuffle2((T)(*(row + xwest)), cells, 
                    w_mask);

                s_cells = vloadn(0, rsouth + x);
                se_cells = shuffle2(s_cells, (T)(*(rsouth + x + VECLEN)),
                    e_mask);
                sw_cells = shuffle2((T)(*(rsouth + xwest)), s_cells, 
                    w_mask);

                vstoren(alive(
                    n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                    s_cells + se_cells + sw_cells, cells 
                ), 0, buf + irow + x);
            }

            // Do the last vector
            n_cells = vloadn(0, rnorth + width - VECLEN);
            nw_cells = shuffle2((T)(*(rnorth + width - VECLEN - 1)),
                n_cells, w_mask);
            ne_cells = shuffle2(n_cells, (T)(*rnorth), 
                e_mask);

            cells = vloadn(0, row + width - VECLEN);
            w_cells = shuffle2((T)(*(row + width - VECLEN - 1)), cells, 
                w_mask);
            e_cells = shuffle2(cells, (T)(*row), 
                e_mask);

            s_cells = vloadn(0, rsouth + width - VECLEN);
            sw_cells = shuffle2((T)(*(rsouth + width - VECLEN - 1)),
                s_cells, w_mask);
            se_cells = shuffle2(s_cells, (T)(*rsouth), 
                e_mask);

            vstoren(alive(
                n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                s_cells + se_cells + sw_cells, cells 
            ), 0, buf + irow + width - VECLEN);
        }
    }
    else {
        for (int y = y_start; y < height; y += w_height) {
            int ynorth = y ? y - 1 : height - 1;
            int ysouth = y == (height - 1) ? 0 : y + 1;
            int irow = y * width;
            int inorth = ynorth * width;
            int isouth = ysouth * width;

            global char* rnorth = grid + inorth;
            global char* row = grid + irow;
            global char* rsouth = grid + isouth;
            
            for (int x = x_start; x < width - VECLEN; x += stride) {
                int xwest = x - 1;
                
                T n_cells = vloadn(0, rnorth + x);
                T ne_cells = shuffle2(n_cells, (T)(*(rnorth + x + 
                    VECLEN)), e_mask);
                T nw_cells = shuffle2((T)(*(rnorth + xwest)), n_cells, 
                    w_mask);

                T cells = vloadn(0, row + x);
                T e_cells = shuffle2(cells, (T)(*(row + x + VECLEN)),
                    e_mask);
                T w_cells = shuffle2((T)(*(row + xwest)), cells, 
                    w_mask);

                T s_cells = vloadn(0, rsouth + x);
                T se_cells = shuffle2(s_cells, (T)(*(rsouth + x + 
                    VECLEN)), e_mask);
                T sw_cells = shuffle2((T)(*(rsouth + xwest)), s_cells, 
                    w_mask);

                vstoren(alive(
                    n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                    s_cells + se_cells + sw_cells, cells 
                ), 0, buf + irow + x);
            }
        }
    }
}