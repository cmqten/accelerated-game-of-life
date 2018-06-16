CXX = g++
CXX_FLAGS = -std=c++17 -Wall -msse2 -mssse3 -fopenmp
LD_FLAGS = -lOpenCL
OPT = -O2 -funroll-loops -funswitch-loops

EXE = generate simulate
OBJ = cell_world.o sim_cpu_sequential.o sim_cpu_simd.o sim_cpu_omp.o \
sim_gpu_ocl.o

all: $(EXE)

$(EXE): %: %.o $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OPT) -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(OPT) -c $^

clean:
	rm -rf $(EXE) $(OBJ) *.o 