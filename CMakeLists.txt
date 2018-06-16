cmake_minimum_required(VERSION 2.8)
project(game_of_life)

get_filename_component(PROJECT_DIR "." REALPATH)
set(SRC_DIR "${PROJECT_DIR}/src")
set(KERNELS_DIR "${SRC_DIR}/kernels")

set(OPTIMIZATIONS "-O2 -funroll-loops -funswitch-loops")
set(COMPILE_FLAGS "-Wall -std=gnu++17 -msse2 -mssse3 -fopenmp")
set(LINKER_FLAGS "-lOpenCL")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATIONS} ${COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

include_directories(./include)
file(GLOB SOURCES "${SRC_DIR}/*.cpp")
add_custom_target(link_kernels ALL ln -s -f ${KERNELS_DIR} 
                  ${CMAKE_CURRENT_BINARY_DIR})           
add_executable(game_of_life ${SOURCES})