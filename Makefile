CC = g++
FLAGS = -Wall -Wextra -std=c++11 -msse2 -mssse3 -fopenmp -O3 -funroll-loops -funswitch-loops -fno-tree-vectorize

SRC_DIR = ./src
INC_DIR = ./include

BUILD_DIR = ./build
CUDA_TO_HIP_DIR = $(BUILD_DIR)/cuda_to_hip
OBJ_DIR = $(BUILD_DIR)/obj

EXECUTABLE = game_of_life
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_SOURCES))

CUDA_SOURCES = $(wildcard $(SRC_DIR)/*.cu)
HIP_SOURCES = $(patsubst $(SRC_DIR)/%.cu, $(CUDA_TO_HIP_DIR)/%.cpp, $(CUDA_SOURCES))
HIP_OBJECTS = $(patsubst $(CUDA_TO_HIP_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(HIP_SOURCES))

all: dir $(BUILD_DIR)/$(EXECUTABLE)

dir:
	mkdir -p $(BUILD_DIR); mkdir -p $(CUDA_TO_HIP_DIR); mkdir -p $(OBJ_DIR)

$(BUILD_DIR)/$(EXECUTABLE): $(CPP_OBJECTS) $(HIP_OBJECTS)
	hipcc $(FLAGS) -I$(INC_DIR) $^ -o $@

$(CPP_OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $(FLAGS) -I$(INC_DIR) $< -o $@

$(HIP_OBJECTS): $(OBJ_DIR)/%.o: $(CUDA_TO_HIP_DIR)/%.cpp
	hipcc -c $(FLAGS) -I$(INC_DIR) $< -o $@

$(HIP_SOURCES): $(CUDA_TO_HIP_DIR)/%.cpp: $(SRC_DIR)/%.cu
	hipify-perl $< > $@

clean:
	rm -rf build
