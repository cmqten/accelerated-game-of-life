constant uchar16 w_mask = (uchar16)(15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
                          25, 26, 27, 28, 29, 30);
constant uchar16 e_mask = (uchar16)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 
                          14, 15, 16);

inline char16 alive(char16 count, char16 state)
{
    return ((count == (char16)(3)) | ((count == (char16)(2)) & state)) & 
        (char16)(1);
}

kernel void life_kernel(global char* grid, global char* buf, int width,
    int height)
{
    int vec_len = sizeof(char16);
    int x_start = get_global_id(0) * vec_len;
    int y_start = get_global_id(1);
    int w_width = get_global_size(0);
    int w_height = get_global_size(1);
    int stride = w_width * vec_len;

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
            char16 n_cells = vload16(0, rnorth);
            char16 ne_cells = shuffle2(n_cells, (char16)(*(rnorth + vec_len)),
                e_mask);
            char16 nw_cells = shuffle2((char16)(*(rnorth + width - 1)), n_cells, 
                w_mask);

            char16 cells = vload16(0, row);
            char16 e_cells = shuffle2(cells, (char16)(*(row + vec_len)),
                e_mask);
            char16 w_cells = shuffle2((char16)(*(row + width - 1)), cells, 
                w_mask);

            char16 s_cells = vload16(0, rsouth);
            char16 se_cells = shuffle2(s_cells, (char16)(*(rsouth + vec_len)),
                e_mask);
            char16 sw_cells = shuffle2((char16)(*(rsouth + width - 1)), s_cells, 
                w_mask);

            vstore16(alive(
                n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                s_cells + se_cells + sw_cells, cells 
            ), 0, buf + irow);

            // Do the middle vectors
            for (int x = stride; x < width - vec_len; x += stride) {
                int xwest = x - 1;

                n_cells = vload16(0, rnorth + x);
                ne_cells = shuffle2(n_cells, (char16)(*(rnorth + x + vec_len)),
                    e_mask);
                nw_cells = shuffle2((char16)(*(rnorth + xwest)), n_cells, 
                    w_mask);

                cells = vload16(0, row + x);
                e_cells = shuffle2(cells, (char16)(*(row + x + vec_len)),
                    e_mask);
                w_cells = shuffle2((char16)(*(row + xwest)), cells, 
                    w_mask);

                s_cells = vload16(0, rsouth + x);
                se_cells = shuffle2(s_cells, (char16)(*(rsouth + x + vec_len)),
                    e_mask);
                sw_cells = shuffle2((char16)(*(rsouth + xwest)), s_cells, 
                    w_mask);

                vstore16(alive(
                    n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                    s_cells + se_cells + sw_cells, cells 
                ), 0, buf + irow + x);
            }

            // Do the last vector
            n_cells = vload16(0, rnorth + width - vec_len);
            nw_cells = shuffle2((char16)(*(rnorth + width - vec_len - 1)),
                n_cells, w_mask);
            ne_cells = shuffle2(n_cells, (char16)(*rnorth), e_mask);

            cells = vload16(0, row + width - vec_len);
            w_cells = shuffle2((char16)(*(row + width - vec_len - 1)), cells, 
                w_mask);
            e_cells = shuffle2(cells, (char16)(*row), e_mask);

            s_cells = vload16(0, rsouth + width - vec_len);
            sw_cells = shuffle2((char16)(*(rsouth + width - vec_len - 1)),
                s_cells, w_mask);
            se_cells = shuffle2(s_cells, (char16)(*rsouth), e_mask);

            vstore16(alive(
                n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                s_cells + se_cells + sw_cells, cells 
            ), 0, buf + irow + width - vec_len);
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
            
            for (int x = x_start; x < width - vec_len; x += stride) {
                int xwest = x - 1;

                char16 n_cells = vload16(0, rnorth + x);
                char16 ne_cells = shuffle2(n_cells, (char16)(*(rnorth + x + 
                    vec_len)), e_mask);
                char16 nw_cells = shuffle2((char16)(*(rnorth + xwest)), n_cells, 
                    w_mask);

                char16 cells = vload16(0, row + x);
                char16 e_cells = shuffle2(cells, (char16)(*(row + x + vec_len)),
                    e_mask);
                char16 w_cells = shuffle2((char16)(*(row + xwest)), cells, 
                    w_mask);

                char16 s_cells = vload16(0, rsouth + x);
                char16 se_cells = shuffle2(s_cells, (char16)(*(rsouth + x + 
                    vec_len)), e_mask);
                char16 sw_cells = shuffle2((char16)(*(rsouth + xwest)), s_cells, 
                    w_mask);

                vstore16(alive(
                    n_cells + ne_cells + nw_cells + e_cells + w_cells + 
                    s_cells + se_cells + sw_cells, cells 
                ), 0, buf + irow + x);
            }
        }
    }
}
