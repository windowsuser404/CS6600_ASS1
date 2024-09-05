# Set the compiler and compiler flags
CC = g++
OPT = -O3
WARN = -Wall
CFLAGS = $(OPT) $(WARN) -I$(INCLUDE_DIR)

# Directories
SRC_DIR = src
INCLUDE_DIR = include

# List all .cpp files in src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

# Generate .o files for each .cpp file in the src directory
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.o)

# Output executable name
TARGET = cache_sim

# Default rule
all: $(TARGET)
	@echo "Build complete!"

# Rule for building the executable
$(TARGET): $(OBJ_FILES)
	$(CC) -o $(TARGET) $(OBJ_FILES) $(CFLAGS)

# Rule for compiling .cpp files to .o files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(wildcard $(INCLUDE_DIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove all compiled files
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

# Phony targets to prevent conflicts with files named 'all', 'clean', etc.
.PHONY: all clean

