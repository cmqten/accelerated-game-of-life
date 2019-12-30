/**
 * life_gpu_ocl.cpp
 *
 * Simulates Conway's Game of Life on a GPU using OpenCL. This simply decides 
 * which kernel to dispatch and how big the global and local workgroup sizes 
 * should be. The kernel does all the calculations.
 * 
 * Author: Carl Marquez
 * Created on: May 26, 2018
 */
#include <CL/cl.hpp>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <libgen.h>
#include <stdexcept>

#include <game_of_life.hpp>
#include <util.hpp>

void gpu_ocl(char* grid, int width, int height, int gens, double* compute_time, double* transfer_in_time,
    double* transfer_out_time)
{
    // Get default device
    cl_int err;
    cl::Device device = cl::Device::getDefault(&err);
    if (err) {
        throw std::runtime_error("No default device found");
    }

    // Create context
    cl::Context context({device});

    // Load kernel source
    char* program_name_copy = strdup(program_invocation_name);
    char* program_dir = dirname(program_name_copy);
    std::string source_path(program_dir);
    source_path += "/gpu_ocl_kernels.cl";
    free(program_name_copy);
    std::ifstream source_file(source_path);
    std::string source_code(std::istreambuf_iterator<char>(source_file), (std::istreambuf_iterator<char>()));

    // Compile kernels
    cl::Program::Sources sources({{source_code.c_str(), source_code.length()}});
    cl::Program program(context, sources);
    if ((err = program.build({device}))) {
        throw std::runtime_error("OpenCL build error " + std::to_string(err) + "\n" + 
            program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) + "\n");
    }

    // Command queue and kernels
    cl::CommandQueue queue(context, device);
    cl::Kernel kernel;

    // Timer
    my_timer timer;

    // Device memory
    int size = width * height;
    cl::Buffer grid_d(context, CL_MEM_READ_WRITE, size);
    cl::Buffer buf_d(context, CL_MEM_READ_WRITE, size);

    // Transfer in
    timer.start();
    queue.enqueueWriteBuffer(grid_d, CL_TRUE, 0, size, grid);
    queue.finish();
    if (transfer_in_time) {
        *transfer_in_time = timer.stop();
    }
    timer.stop();

    // Kernel global and local sizes
    int max_work_group_size = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    int max_compute_units = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();  
    int global_width = 0;
    int global_height = 0;

    if (width == 16 || width == 8 || width == 4) {
        global_width = 1;
        global_height = nearest_le_pow_2(height);
        
        if (width == 16) {
            kernel = cl::Kernel(program, "kernel_width_16");
        }
        else if (width == 8) {
            kernel = cl::Kernel(program, "kernel_width_8");
        }
        else {
            kernel = cl::Kernel(program, "kernel_width_4");
        }
    }
    else if (width > 16) {
        global_width = nearest_le_pow_2((width + 15) / 16);
        global_height = nearest_le_pow_2(height);
        kernel = cl::Kernel(program, "kernel_width_gt16");
    }
    cl::NDRange global_group(global_width, global_height);

    kernel.setArg<int>(2, width);
    kernel.setArg<int>(3, height);

    // Launch kernel for every generation
    timer.start();
    for (int i = 0; i < gens / 2; ++i) {
        kernel.setArg<cl::Buffer>(0, grid_d);
        kernel.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_group);
        kernel.setArg<cl::Buffer>(0, buf_d);
        kernel.setArg<cl::Buffer>(1, grid_d);
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_group);
    }
    if (gens & 1) {
        kernel.setArg<cl::Buffer>(0, grid_d);
        kernel.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_group);
    }
    queue.finish();
    if (compute_time) {
        *compute_time = timer.stop();
    }
    timer.stop();

    // Transfer out
    timer.start();
    if (gens & 1) {
        queue.enqueueReadBuffer(buf_d, CL_TRUE, 0, size, grid);
    }
    else {
        queue.enqueueReadBuffer(grid_d, CL_TRUE, 0, size, grid);
    }
    queue.finish();
    if (transfer_out_time) {
        *transfer_out_time = timer.stop();
    }
    timer.stop();
}