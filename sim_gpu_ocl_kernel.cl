kernel void gpu_life(global char* grid, global char* buf, int width, int height)
{
    int tid = get_global_id(0);
    int wsize = get_global_size(0);
    int size = width * height;

    for (int i = tid; i < size; i += wsize) {
        int x = i % width;
        int y = i / width;

        int ynorth = y ? y - 1 : height - 1;
        int ysouth = y == (height - 1) ? 0 : y + 1;
        int xwest = x ? x - 1 : width - 1;
        int xeast = x == (width - 1) ? 0 : x + 1;

        int inorth = ynorth * width;
        int irow = y * width;
        int isouth = ysouth * width;
        int cell = grid[inorth + xwest] + grid[inorth + x] + grid[inorth + xeast] +
                grid[irow + xwest] + grid[irow + xeast] + grid[isouth + xwest] + 
                grid[isouth + x] + grid[isouth + xeast];
        cell = (cell == 3) | ((cell == 2) & grid[i]);
        buf[i] = cell;
    }
}