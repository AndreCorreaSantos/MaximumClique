# Compiler
CXX = g++
NVCC = nvcc

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -fopenmp
NVCCFLAGS = -ccbin /usr/bin/g++-10 -I /usr/include/c++/10 -arch=sm_86 

# Source files
CPP_SOURCES = $(wildcard *.cpp)
CU_SOURCES = $(wildcard *.cu)

# Executables
CPP_EXECUTABLES = $(CPP_SOURCES:.cpp=)
CU_EXECUTABLES = $(CU_SOURCES:.cu=)

# Default rule
all: $(CPP_EXECUTABLES) $(CU_EXECUTABLES)

# Rule to compile .cpp files into executables
%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Rule to compile .cu files into executables
%: %.cu
	$(NVCC) $(NVCCFLAGS) -o $@ $<

# Clean rule
clean:
	rm -f $(CPP_EXECUTABLES) $(CU_EXECUTABLES)
