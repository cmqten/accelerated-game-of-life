#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include "sim.hpp"
#include "util.hpp"

void sim_gpu_ocl(char* grid, int width, int height, int gens)
{
    my_timer timer;
    // Get default device
    cl_int err;
    cl::Device device = cl::Device::getDefault(&err);
    throw_false<std::runtime_error>(err == CL_SUCCESS, 
        "No default device found");

    // Create context
    cl::Context context({device});

    // Build source
    std::ifstream source_file("sim_gpu_ocl_kernel.cl");
    std::string source_code(std::istreambuf_iterator<char>(source_file),
        (std::istreambuf_iterator<char>()));

    cl::Program::Sources sources;
    sources.push_back({source_code.c_str(), source_code.length()});
    cl::Program program(context, sources);
    throw_false<std::runtime_error>(program.build({device}) == CL_SUCCESS,
        "OpenCL build error\n" + 
        program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) + "\n");

    // Command queue and kernels
    cl::CommandQueue queue(context, device);
    cl::Kernel gpu_life(program, "gpu_life");

    // Device memory
    int size = width * height;
    cl::Buffer grid_d(context, CL_MEM_READ_WRITE, size);
    cl::Buffer buf_d(context, CL_MEM_READ_WRITE, size);

    // Transfer from host to device memory
    
    queue.enqueueWriteBuffer(grid_d, CL_TRUE, 0, size, grid);

    // Launch kernel for every generation
    timer.start();
    gpu_life.setArg<int>(2, width);
    gpu_life.setArg<int>(3, height);

    for (int i = 0; i < gens / 2; ++i) {
        gpu_life.setArg<cl::Buffer>(0, grid_d);
        gpu_life.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(gpu_life, cl::NullRange, cl::NDRange(1024),
            cl::NDRange(32));
        gpu_life.setArg<cl::Buffer>(0, buf_d);
        gpu_life.setArg<cl::Buffer>(1, grid_d);
        queue.enqueueNDRangeKernel(gpu_life, cl::NullRange, cl::NDRange(1024),
            cl::NDRange(32));
    }
    if (gens & 1) {
        gpu_life.setArg<cl::Buffer>(0, grid_d);
        gpu_life.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(gpu_life, cl::NullRange, cl::NDRange(1024),
            cl::NDRange(32));
        queue.finish();
        std::cout << timer.stop() << std::endl;
        queue.enqueueReadBuffer(buf_d, CL_TRUE, 0, size, grid);
    }
    else {
        queue.finish();
        std::cout << timer.stop() << std::endl;
        queue.enqueueReadBuffer(grid_d, CL_TRUE, 0, size, grid);
    }
    queue.finish();
}