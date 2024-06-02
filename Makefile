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

# Executable names derived from source files
CPP_EXECUTABLES = $(CPP_SOURCES:.cpp=)
CU_EXECUTABLES = $(CU_SOURCES:.cu=)
MPI_EXECUTABLES = $(filter %_mpi, $(CPP_EXECUTABLES))

# Default target
all: $(CPP_EXECUTABLES) $(CU_EXECUTABLES)

# Rule to make MPI executables
%_mpi: %_mpi.cpp
	$(MPI_CXX) $(MPIFLAGS) -o $@ $<

# Rule to make standard executables
%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Rule to compile CUDA files
%: %.cu
	$(NVCC) $(NVCCFLAGS) -o $@ $<

# Clean target
clean:
	rm -f $(CPP_EXECUTABLES) $(CU_EXECUTABLES) $(MPI_EXECUTABLES)
