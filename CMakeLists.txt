cmake_minimum_required(VERSION 2.8)
project(game_of_life)

get_filename_component(PROJECT_DIR "." REALPATH)
set(SRC_DIR "${PROJECT_DIR}/src")
set(KERNELS_DIR "${SRC_DIR}/kernels")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=gnu++17 -msse2 -mssse3 -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -funroll-loops -funswitch-loops")

include_directories(./include)
link_directories($ENV{OCLDIR})

file(GLOB SOURCES "${SRC_DIR}/*.cpp")
add_custom_target(link_kernels ALL ln -s -f ${KERNELS_DIR} 
                  ${CMAKE_CURRENT_BINARY_DIR})           
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} OpenCL stdc++fs)