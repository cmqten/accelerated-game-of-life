CXX = g++
CXX_FLAGS = -std=c++17 -Wall -Wno-unused-function
LD_FLAGS = 

EXE = generate
OPT = -O2
OBJ = game_of_life.o

all: $(EXE)

$(EXE): %: %.o $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OPT) -o $@ $^ $(LD_FLAGS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $(OPT) -c $^

clean:
	rm -rf $(EXE) $(OBJ) *.o 