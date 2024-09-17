# Set the compiler and compiler flags
CC = g++
OPT = -O3
WARN = -Wall
CFLAGS = $(OPT) $(WARN) -I$(INCLUDE_DIR) -g

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# List all .cpp files in src directory
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

# Generate corresponding .o files in the build directory
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Output executable name
TARGET = cache_sim

# Default rule
all: $(TARGET)
	@echo "Build complete!"

run: $(TARGET)
	@echo "Build complete!"
	# reset
	./TEST.sh > ./run.log

# Rule for building the executable
$(TARGET): $(OBJ_FILES)
	$(CC) -o $(TARGET) $(OBJ_FILES) $(CFLAGS)

# Rule for compiling .cpp to .o, ensuring the build directory exists
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove all compiled files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Phony targets to avoid conflicts
.PHONY: all clean

