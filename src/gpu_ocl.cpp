/**
 * gpu_ocl.cpp
 *
 * Simulates Conway's Game of Life on a GPU using OpenCL.
 * 
 * Author: Carl Marquez
 * Created on: May 26, 2018
 */
#include <algorithm>
#include <CL/cl.hpp>
#include <iostream>

#include <game_of_life.hpp>
#include <gpu_ocl.hpp>
#include <util.hpp>

gpu_ocl_compiler compiler;
const int compute_units = compiler.device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
const int max_local_size = compiler.device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
const int processors_per_cu = 64; // AMD GCN
const int processors_total = processors_per_cu * compute_units;
const int workgroups_per_cu = 2; // Arbitrary limit
const int workgroups_total = workgroups_per_cu * compute_units;

/* Returns kernel function name, global dimensions, and local dimensions for
given world size. */
void get_kernel_launch_params(int width, int height, std::string& kernel_func, int& global_width, int& global_height,
    int& local_width, int& local_height)
{
    int size = width * height;

    if (width == 16 || width == 8 || width == 4) {
        global_width = 1;
        local_width = 1;

        // If number of cells is divisible by 16 but width is 4 or 8, process
        // multiple rows together as vector of 16 cells.
        if (!(size % 16) && width < 16) {
            int local_size = processors_per_cu;
            int processors_needed = height / (16 / width);
            if (processors_needed > processors_total) {
                local_size = (processors_needed + compute_units - 1) / compute_units;
                local_size = std::min((local_size / processors_per_cu) * processors_per_cu, max_local_size);
                local_size = nearest_le_pow_2(local_size);
            }
            int num_groups = std::min(workgroups_total, (processors_needed + local_size - 1) / local_size);

            global_height = num_groups * local_size;
            local_height = local_size;
            kernel_func.assign("kernel_width_" + std::to_string(width) + "_cells_mul16");
        }
        else {
            int local_size = processors_per_cu;
            if (height > processors_total) {
                local_size = (height + compute_units - 1) / compute_units;
                local_size = std::min((local_size / processors_per_cu) * processors_per_cu, max_local_size);
                local_size = nearest_le_pow_2(local_size);
            }
            int num_groups = std::min(workgroups_total, (height + local_size - 1) / local_size);

            global_height = num_groups * local_size;
            local_height = local_size;
            kernel_func.assign("kernel_width_" + std::to_string(width));
        }
    }
    else if (width > 16 && is_power_of_2(width)) {
        int processors_needed = size / 16;
        int _global_width = width / 16;

        // Workgroup size is power of 2 ranging from processors per CU to 
        // maximum workgroup size.
        int local_size = processors_per_cu;
        if (processors_needed > processors_total) {
            local_size = (processors_needed + compute_units - 1) / compute_units;
            local_size = std::min((local_size / processors_per_cu) * processors_per_cu, max_local_size);
            local_size = nearest_le_pow_2(local_size);
        }
        int _local_width = std::min(_global_width, local_size);
        int _local_height = local_size / _local_width;
        int workgroups_per_row = _global_width / _local_width;

        // If number of workgroups needed if processing 16 cells per processor
        // larger than workgroups total, limit to smallest number larger or 
        // equal to workgroups_total that forms rectangular global dimensions.
        // Each processor may processor more than 16 cells.
        global_height = std::min(((height + _local_height - 1) / _local_height), 
            (workgroups_total + workgroups_per_row - 1) / workgroups_per_row) * _local_height;
        global_width = _global_width;
        local_height = _local_height;
        local_width = _local_width;
        kernel_func.assign("kernel_width_gt16_pow2");
    }
}

void gpu_ocl(char* grid, int width, int height, int gens, double* compute_time, double* transfer_in_time,
    double* transfer_out_time)
{
    cl::Kernel kernel;
    my_timer timer;

    // Device memory
    int size = width * height;
    cl::Buffer grid_d(compiler.context, CL_MEM_READ_WRITE, size);
    cl::Buffer buf_d(compiler.context, CL_MEM_READ_WRITE, size);

    // Transfer in
    timer.start();
    compiler.queue.enqueueWriteBuffer(grid_d, CL_TRUE, 0, size, grid);
    compiler.queue.finish();
    if (transfer_in_time) {
        *transfer_in_time = timer.stop();
    }
    timer.stop();

    // Kernel function name, global and work group sizes
    std::string kernel_func;
    int global_width = 0;
    int global_height = 0;
    int local_width = 0;
    int local_height = 0;

    // Global and workgroup sizes
    get_kernel_launch_params(width, height, kernel_func, global_width, global_height, local_width, local_height);
    kernel = cl::Kernel(compiler.program, kernel_func.c_str());
    cl::NDRange global_size(global_width, global_height);
    cl::NDRange local_size(local_width, local_height);

    std::cout << global_width << " " << global_height << " " << local_width << " " << local_height << std::endl;

    kernel.setArg<int>(2, width);
    kernel.setArg<int>(3, height);

    // Launch kernel for every generation
    timer.start();
    for (int i = 0; i < gens / 2; ++i) {
        kernel.setArg<cl::Buffer>(0, grid_d);
        kernel.setArg<cl::Buffer>(1, buf_d);
        compiler.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size);
        kernel.setArg<cl::Buffer>(0, buf_d);
        kernel.setArg<cl::Buffer>(1, grid_d);
        compiler.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size);
    }
    if (gens & 1) {
        kernel.setArg<cl::Buffer>(0, grid_d);
        kernel.setArg<cl::Buffer>(1, buf_d);
        compiler.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_size, local_size);
    }
    compiler.queue.finish();
    if (compute_time) {
        *compute_time = timer.stop();
    }
    timer.stop();

    // Transfer out
    timer.start();
    if (gens & 1) {
        compiler.queue.enqueueReadBuffer(buf_d, CL_TRUE, 0, size, grid);
    }
    else {
        compiler.queue.enqueueReadBuffer(grid_d, CL_TRUE, 0, size, grid);
    }
    compiler.queue.finish();
    if (transfer_out_time) {
        *transfer_out_time = timer.stop();
    }
    timer.stop();
}
