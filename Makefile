# Compilers
CXX = g++
MPI_CXX = mpicxx
NVCC = nvcc

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -fopenmp
MPIFLAGS = $(CXXFLAGS)
NVCCFLAGS = -ccbin /usr/bin/g++-10 -I /usr/include/c++/10 -arch=sm_86 --extended-lambda

# Source files
CPP_SOURCES = $(wildcard *.cpp)
CU_SOURCES = $(wildcard *.cu)

# Filter sources containing the main function
MAIN_SOURCES = $(shell grep -l 'int main' $(CPP_SOURCES))

# Separate MPI sources for special handling
MPI_SOURCES = $(filter %_mpi.cpp, $(MAIN_SOURCES))
NON_MPI_SOURCES = $(filter-out %_mpi.cpp, $(MAIN_SOURCES))

# Executable names derived from source files
CPP_EXECUTABLES = $(NON_MPI_SOURCES:.cpp=) $(MPI_SOURCES:.cpp=)
CU_EXECUTABLES = $(CU_SOURCES:.cu=)

# All other source files without main (assuming they contain helper functions)
HELPER_SOURCES = $(filter-out $(MAIN_SOURCES), $(CPP_SOURCES))

# Default target
all: $(CPP_EXECUTABLES) $(CU_EXECUTABLES)

# Rule to make standard executables
$(NON_MPI_SOURCES:.cpp=): %: %.cpp $(HELPER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to make MPI executables
$(MPI_SOURCES:.cpp=): %: %.cpp $(HELPER_SOURCES)
	$(MPI_CXX) $(MPIFLAGS) -o $@ $^

# Rule to compile CUDA files
$(CU_EXECUTABLES): %: %.cu
	$(NVCC) $(NVCCFLAGS) -o $@ $<

# Clean target
clean:
	rm -f $(CPP_EXECUTABLES) $(CU_EXECUTABLES)
