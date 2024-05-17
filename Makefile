# Compiler to use
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -fopenmp

# Find all .cpp files
SOURCES = $(wildcard *.cpp)

# Filter sources containing the main function
MAIN_SOURCES = $(shell grep -l 'int main' $(SOURCES))

# Convert .cpp files to executable names
EXECUTABLES = $(MAIN_SOURCES:.cpp=)

# All other source files without main (assuming they contain helper functions)
HELPER_SOURCES = $(filter-out $(MAIN_SOURCES), $(SOURCES))

# Default target
all: $(EXECUTABLES)

# Rule to make executables
%: %.cpp $(HELPER_SOURCES)
	$(CXX) $(CXXFLAGS) $< $(HELPER_SOURCES) -o $@

# Clean target
clean:
	rm -f $(EXECUTABLES)
