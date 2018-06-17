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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <life.hpp>
#include <util.hpp>

extern char* path;

void life_gpu_ocl(char* grid, int width, int height, int gens)
{
    int vec_len = 2;
    std::string source_path(std::filesystem::path(path).parent_path().c_str());
    source_path += "/kernels/";
    if (width == 16 || width == 8 || width == 4) {
        source_path += "life_ocl_kernel_e.cl";
        vec_len = width;
    }
    else {
        source_path += "life_ocl_kernel.cl";
        if (width > 16) vec_len = 16;
        else if (width > 8) vec_len = 8;
        else if (width > 4) vec_len = 4;
        else vec_len = 2;
    }

    // Get default device
    cl_int err;
    cl::Device device = cl::Device::getDefault(&err);
    throw_false<std::runtime_error>(err == CL_SUCCESS, "No default found");

    // Create context
    cl::Context context({device});

    std::ifstream source_file(source_path);
    std::string source_code(std::istreambuf_iterator<char>(source_file),
        (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources;
    sources.push_back({source_code.c_str(), source_code.length()});
    cl::Program program(context, sources);
    std::string compile_flags = "-DVECLEN=" + std::to_string(vec_len);
    err = program.build({device}, compile_flags.c_str());
    throw_false<std::runtime_error>(err == CL_SUCCESS,
        "OpenCL build error " + std::to_string(err) + "\n" + 
        program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) + "\n");

    // Command queue and kernels
    cl::CommandQueue queue(context, device);
    cl::Kernel life_kernel(program, "life_kernel");

    // Device memory
    int size = width * height;
    cl::Buffer grid_d(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
        (size_t)size, grid);
    cl::Buffer buf_d(context, CL_MEM_READ_WRITE, size);

    // Kernel global and local sizes
    cl::NDRange global_group(128, 128);
    cl::NDRange local_group(16, 16);

    // Launch kernel for every generation
    life_kernel.setArg<int>(2, width);
    life_kernel.setArg<int>(3, height);

    for (int i = 0; i < gens / 2; ++i) {
        life_kernel.setArg<cl::Buffer>(0, grid_d);
        life_kernel.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(life_kernel, cl::NullRange, global_group,
            local_group);
        life_kernel.setArg<cl::Buffer>(0, buf_d);
        life_kernel.setArg<cl::Buffer>(1, grid_d);
        queue.enqueueNDRangeKernel(life_kernel, cl::NullRange, global_group,
            local_group);
    }
    if (gens & 1) {
        life_kernel.setArg<cl::Buffer>(0, grid_d);
        life_kernel.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(life_kernel, cl::NullRange, global_group,
            local_group);
        queue.enqueueReadBuffer(buf_d, CL_TRUE, 0, size, grid);
    }
    else {
        queue.enqueueReadBuffer(grid_d, CL_TRUE, 0, size, grid);
    }
    queue.finish();
}