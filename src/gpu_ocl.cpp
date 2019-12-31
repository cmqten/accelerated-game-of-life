/**
 * gpu_ocl.cpp
 *
 * Simulates Conway's Game of Life on a GPU using OpenCL.
 * 
 * Author: Carl Marquez
 * Created on: May 26, 2018
 */
#include <CL/cl.hpp>

#include <game_of_life.hpp>
#include <gpu_ocl.hpp>
#include <util.hpp>

gpu_ocl_compiler compiler;

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

    // Kernel global and local sizes
    int max_work_group_size = compiler.device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    int max_compute_units = compiler.device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();  
    int global_width = 0;
    int global_height = 0;

    if (width == 16 || width == 8 || width == 4) {
        global_width = 1;
        global_height = nearest_le_pow_2(height);
        
        if (width == 16) {
            kernel = cl::Kernel(compiler.program, "kernel_width_16");
        }
        else if (width == 8) {
            kernel = cl::Kernel(compiler.program, "kernel_width_8");
        }
        else {
            kernel = cl::Kernel(compiler.program, "kernel_width_4");
        }
    }
    else if (width > 16) {
        global_width = nearest_le_pow_2((width + 15) / 16);
        global_height = nearest_le_pow_2(height);
        kernel = cl::Kernel(compiler.program, "kernel_width_gt16");
    }
    cl::NDRange global_group(global_width, global_height);

    kernel.setArg<int>(2, width);
    kernel.setArg<int>(3, height);

    // Launch kernel for every generation
    timer.start();
    for (int i = 0; i < gens / 2; ++i) {
        kernel.setArg<cl::Buffer>(0, grid_d);
        kernel.setArg<cl::Buffer>(1, buf_d);
        compiler.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_group);
        kernel.setArg<cl::Buffer>(0, buf_d);
        kernel.setArg<cl::Buffer>(1, grid_d);
        compiler.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_group);
    }
    if (gens & 1) {
        kernel.setArg<cl::Buffer>(0, grid_d);
        kernel.setArg<cl::Buffer>(1, buf_d);
        compiler.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_group);
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