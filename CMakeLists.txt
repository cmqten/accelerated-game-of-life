cmake_minimum_required(VERSION 2.8)
project(game_of_life)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -msse2 -mssse3 -msse4.1 -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -funswitch-loops -fno-tree-vectorize")

get_filename_component(PROJECT_DIR "." REALPATH)
set(SRC_DIR "${PROJECT_DIR}/src")

add_custom_target(link_kernels ALL ln -s -f ${SRC_DIR}/gpu_ocl_kernels.cl ${CMAKE_CURRENT_BINARY_DIR}/gpu_ocl_kernels.cl)       
include_directories(./include)
file(GLOB SOURCES "${SRC_DIR}/*.cpp")         
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} OpenCL)