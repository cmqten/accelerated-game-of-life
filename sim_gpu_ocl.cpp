#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include "sim.hpp"
#include "util.hpp"

void sim_gpu_ocl(char* grid, int width, int height, int gens)
{
    // Get default device
    cl_int err;
    cl::Device device = cl::Device::getDefault(&err);
    throw_false<std::runtime_error>(err == CL_SUCCESS, "No default found");

    // Create context
    cl::Context context({device});

    // Build source
    std::ifstream source_file("sim_gpu_ocl_kernel.cl");
    std::string source_code(std::istreambuf_iterator<char>(source_file),
        (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources;
    sources.push_back({source_code.c_str(), source_code.length()});
    cl::Program program(context, sources);
    err = program.build({device});
    throw_false<std::runtime_error>(err == CL_SUCCESS,
        "OpenCL build error " + std::to_string(err) + "\n" + 
        program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) + "\n");

    // Command queue and kernels
    cl::CommandQueue queue(context, device);
    cl::Kernel gpu_life(program, "life_kernel_simd8");

    // Device memory
    int size = width * height;
    cl::Buffer grid_d(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
        static_cast<size_t>(size), grid);
    cl::Buffer buf_d(context, CL_MEM_READ_WRITE, size);

    my_timer timer;
    timer.start();
    // Launch kernel for every generation
    gpu_life.setArg<int>(2, width);
    gpu_life.setArg<int>(3, height);

    int global_w = 256;
    int global_h = 256;
    int wg_w = 16;
    int wg_h = 16;

    for (int i = 0; i < gens / 2; ++i) {
        gpu_life.setArg<cl::Buffer>(0, grid_d);
        gpu_life.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(gpu_life, cl::NullRange, 
            cl::NDRange(global_w, global_h), cl::NDRange(wg_w, wg_h));
        gpu_life.setArg<cl::Buffer>(0, buf_d);
        gpu_life.setArg<cl::Buffer>(1, grid_d);
        queue.enqueueNDRangeKernel(gpu_life, cl::NullRange, 
            cl::NDRange(global_w, global_h), cl::NDRange(wg_w, wg_h));
    }
    if (gens & 1) {
        gpu_life.setArg<cl::Buffer>(0, grid_d);
        gpu_life.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(gpu_life, cl::NullRange, 
            cl::NDRange(global_w, global_h), cl::NDRange(wg_w, wg_h));
        queue.finish();
        std::cout << timer.stop() << " : ";
        queue.enqueueReadBuffer(buf_d, CL_TRUE, 0, size, grid);
    }
    else {
        queue.finish();
        std::cout << timer.stop() << " : ";
        queue.enqueueReadBuffer(grid_d, CL_TRUE, 0, size, grid);
    }
    queue.finish();
}