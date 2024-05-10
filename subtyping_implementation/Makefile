# Compiler options
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
CXXFLAGS_RELEASE = -std=c++17 -Wall -O2

# Directories
SRC_DIR = src
INC_DIR = header
BUILD_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Header files
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

# Target executable
TARGET = $(BUILD_DIR)/main

# Default target
default: $(TARGET)

# Release target
release: CXXFLAGS = $(CXXFLAGS_RELEASE)
release: $(TARGET)

# Rule to link object files into executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: default clean release
