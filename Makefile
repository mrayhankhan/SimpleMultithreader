# List of executables to be generated
EXE=vector matrix

# Default target to build all executables
all: clean $(EXE)

# Rule to compile .cpp files into executables
%: %.cpp
	g++ -O3 -std=c++11 -o $@ $^ -lpthread

# Target to clean up generated files
clean:
	rm -rf $(EXE) 2>/dev/null
