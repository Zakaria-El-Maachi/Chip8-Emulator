# Variables
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRCS = main.cpp
OBJ = $(SRCS:.cpp=.o)

# The name of your executable
TARGET = chip8_emulator

# Default rule: Link the object files into the executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

# Rule to compile .cpp files into .o (object) files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(TARGET) *.o