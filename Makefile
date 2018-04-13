CXX=g++
CXX_FLAGS=-Wall -O2

all: generate

generate: generate.o
	$(CXX) $(CXX_FLAGS) -o $@ $?

%.o : %.cpp
	$(CXX) $(CXX_FLAGS) -c $?

clean:
	rm -rf generate *.o 