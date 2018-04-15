CXX = g++
CXX_FLAGS = -std=c++17 -Wall -Wno-unused-function -g
LD_FLAGS = 

EXE = generate board_test
OPT = -O2
OBJ = board.o unit_test.o gol_cpu.o

all: $(EXE)

$(EXE): %: %.o $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OPT) -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(OPT) -c $^

clean:
	rm -rf $(EXE) $(OBJ) *.o 