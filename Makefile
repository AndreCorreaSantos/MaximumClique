# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++11 -Wall -Wextra
# Source files
SRCS = first.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Executable name
TARGET = first

# Build rule for the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET) grafo.txt

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)
