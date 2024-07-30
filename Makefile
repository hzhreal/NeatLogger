# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Iinclude

# Source directory
SRC_DIR = source

# Include directory
INC_DIR = include

# Object directory
OBJ_DIR = obj

# Library directory
LIB_DIR = lib

# Test directory
TEST_DIR = test

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Library name
LIB_NAME = libNeatLogger.a

# Target
TARGET = $(LIB_DIR)/$(LIB_NAME)

# Test source file
TEST_SRC = $(TEST_DIR)/test.c

# Test object file
TEST_OBJ = $(TEST_DIR)/test.o

# Test executable
TEST_EXEC = $(TEST_DIR)/test

# Rule to compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build library
$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	ar rcs $@ $^

# Rule to build test executable
$(TEST_EXEC): $(TEST_OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJ) -L$(LIB_DIR) -lNeatLogger

.PHONY: all clean test

# Build all
all: $(TARGET)

# Build test executable
test: $(TEST_EXEC)

# Clean objects and library
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(TEST_OBJ) $(TEST_EXEC)
