cmake_minimum_required(VERSION 2.8)
project(game_of_life)

set(OPTIMIZATIONS "-O2 -funroll-loops -funswitch-loops")
set(COMPILE_FLAGS "-Wall -std=gnu++17 -msse2 -mssse3 -fopenmp")
set(LINKER_FLAGS "-lOpenCL")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATIONS} ${COMPILE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

include_directories(./include)
file(COPY "src/kernels" DESTINATION ".")
file(GLOB SOURCES "src/*.cpp")
add_executable(simulate ${SOURCES})