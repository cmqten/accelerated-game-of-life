#include <game_of_life.hpp>
#include <util.hpp>

__global__ void gpu_cuda(char* world, char* buf, int width, int height)
{
    int start_y = blockIdx.x * blockDim.y + threadIdx.y;
    int stride = gridDim.x * blockDim.y;
    int x = threadIdx.x;

    for (int y = start_y; y < height; y += stride) {
        int idx = y * width + x;
        int current_cell = world[idx];

        int x_west = x ? x - 1 : width - 1;
        int x_east = x == width - 1 ? 0 : x + 1;
        int y_north = y ? y - 1 : height - 1;
        int y_south = y == height - 1 ? 0 : y + 1;

        char* north_row_ptr = world + (y_north * width);
        char* current_row_ptr = world + (y * width);
        char* south_row_ptr = world + (y_south * width);

        int neighbors = north_row_ptr[x_east] + north_row_ptr[x] + north_row_ptr[x_west] +
                        current_row_ptr[x_east] + current_row_ptr[x_west] +
                        south_row_ptr[x_east] + south_row_ptr[x] + south_row_ptr[x_west];
        int new_cell = ((neighbors == 3) || (current_cell && neighbors == 2)) & 1;
        buf[idx] = new_cell;
    }
}

double run_game_of_life_gpu(char* world, int width, int height, int gens)
{
    int size = width * height;
    char* world_d;
    char* buf_d;
    cudaMalloc((void**)&world_d, size);
    cudaMalloc((void**)&buf_d, size);
    cudaMemcpy(world_d, world, size, cudaMemcpyHostToDevice);
    cudaDeviceSynchronize();

    int max_threads_per_block = 1024;
    int block_width = width;
    int block_height = 0;

    if (width > max_threads_per_block / 2) {
        block_height = 1;
    }
    else {
        block_height = max_threads_per_block / width;
    }

    dim3 dimBlock(block_width, block_height);

    my_timer timer;
    timer.start();
    for (int i = 0; i < gens / 2; i++) {
        gpu_cuda<<<36, dimBlock>>>(world_d, buf_d, width, height);
        gpu_cuda<<<36, dimBlock>>>(buf_d, world_d, width, height);
    }
    if (gens % 2) {
        gpu_cuda<<<36, dimBlock>>>(world_d, buf_d, width, height);
    }
    cudaDeviceSynchronize();

    if (gens % 2) {
        cudaMemcpy(world, buf_d, size, cudaMemcpyDeviceToHost);
    }
    else {
        cudaMemcpy(world, world_d, size, cudaMemcpyDeviceToHost);
    }
    cudaDeviceSynchronize();
    return timer.stop();
}
