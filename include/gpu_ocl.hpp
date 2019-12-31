/**
 * gpu_ocl.hpp
 *
 * Simulates Conway's Game of Life on a GPU using OpenCL.
 * 
 * Author: Carl Marquez
 * Created on: December 30, 2019
 */
#include <CL/cl.hpp>
#include <errno.h>
#include <fstream>
#include <libgen.h>
#include <stdexcept>

class gpu_ocl_compiler 
{
public:
    cl::Context context;
    cl::Device device;
    cl::Program program;
    cl::CommandQueue queue;
    cl::Program::Sources sources;

    inline gpu_ocl_compiler() 
    {
        // Get default device
        cl_int err;
        device = cl::Device::getDefault(&err);
        if (err) {
            throw std::runtime_error("No default device found");
        }

        // Create context
        context = cl::Context({device});

        // Load kernel source
        char* program_name_copy = strdup(program_invocation_name);
        char* program_dir = dirname(program_name_copy);
        std::string source_path(program_dir);
        source_path += "/gpu_ocl_kernels.cl";
        free(program_name_copy);
        std::ifstream source_file(source_path);
        std::string source_code(std::istreambuf_iterator<char>(source_file), (std::istreambuf_iterator<char>()));

        // Compile kernels
        sources = cl::Program::Sources({{source_code.c_str(), source_code.length()}});
        program = cl::Program(context, sources);
        if ((err = program.build({device}))) {
            throw std::runtime_error("OpenCL build error " + std::to_string(err) + "\n" + 
                program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) + "\n");
        }

        // Command queue and kernels
        queue = cl::CommandQueue(context, device);
    };
};