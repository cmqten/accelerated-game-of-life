#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
#include <life.hpp>
#include <util.hpp>

void life_gpu_ocl(char* grid, int width, int height, int gens)
{
    // Get default device
    cl_int err;
    cl::Device device = cl::Device::getDefault(&err);
    throw_false<std::runtime_error>(err == CL_SUCCESS, "No default found");

    // Create context
    cl::Context context({device});

    // Build source
    std::string source_path = "kernels/life_ocl_kernel_8.cl";
    std::ifstream source_file(source_path);
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
    cl::Kernel sim_gpu_ocl_kernel(program, "life_kernel");

    // Device memory
    int size = width * height;
    cl::Buffer grid_d(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
        (size_t)size, grid);
    cl::Buffer buf_d(context, CL_MEM_READ_WRITE, size);

    my_timer timer;
    timer.start();

    // Launch kernel for every generation
    sim_gpu_ocl_kernel.setArg<int>(2, width);
    sim_gpu_ocl_kernel.setArg<int>(3, height);

    int global_w = 128;
    int global_h = 128;
    int wg_w = 16;
    int wg_h = 16;

    for (int i = 0; i < gens / 2; ++i) {
        sim_gpu_ocl_kernel.setArg<cl::Buffer>(0, grid_d);
        sim_gpu_ocl_kernel.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(sim_gpu_ocl_kernel, cl::NullRange, 
            cl::NDRange(global_w, global_h), cl::NDRange(wg_w, wg_h));
        sim_gpu_ocl_kernel.setArg<cl::Buffer>(0, buf_d);
        sim_gpu_ocl_kernel.setArg<cl::Buffer>(1, grid_d);
        queue.enqueueNDRangeKernel(sim_gpu_ocl_kernel, cl::NullRange, 
            cl::NDRange(global_w, global_h), cl::NDRange(wg_w, wg_h));
    }
    if (gens & 1) {
        sim_gpu_ocl_kernel.setArg<cl::Buffer>(0, grid_d);
        sim_gpu_ocl_kernel.setArg<cl::Buffer>(1, buf_d);
        queue.enqueueNDRangeKernel(sim_gpu_ocl_kernel, cl::NullRange, 
            cl::NDRange(global_w, global_h), cl::NDRange(wg_w, wg_h));
        queue.finish();
        std::cout << timer.stop();
        queue.enqueueReadBuffer(buf_d, CL_TRUE, 0, size, grid);
    }
    else {
        queue.finish();
        std::cout << timer.stop();
        queue.enqueueReadBuffer(grid_d, CL_TRUE, 0, size, grid);
    }
    queue.finish();
}