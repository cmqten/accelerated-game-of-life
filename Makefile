CXX = g++
CXX_FLAGS = -std=c++17 -Wall
LD_FLAGS = -lstdc++fs
OPT = -O2
OBJ = board.o

all: generate

generate: generate.o $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OPT) -o $@ $^ $(LD_FLAGS)

%.o : %.cpp
	$(CXX) $(CXX_FLAGS) $(OPT) -c $^

clean:
	rm -rf generate $(OBJ) *.o 