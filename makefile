# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -g

# Directories
SRC_DIR = src
BUILD_DIR = build

# Find all source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Convert source files to object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Output executable
TARGET = tuf_fs

# Default target
all: $(TARGET)

# Link object files
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

.PHONY: all clean rebuild