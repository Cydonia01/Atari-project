# Makefile for Virtual Video Game Console

# Compiler
CC = gcc

# Source files
SRC_DIR = src
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Executable output directory
BIN_DIR = bin
EXE_FILES = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%, $(SRC_FILES))

# Default target: build all
all: $(BIN_DIR) $(EXE_FILES)

# Rule to create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Rule to compile each C file into an executable
$(BIN_DIR)/%: $(SRC_DIR)/%.c
	$(CC) -o $@ $<

# Clean up generated files
clean:
	rm -rf $(BIN_DIR)

# Phony targets to avoid conflicts with files
.PHONY: all clean
