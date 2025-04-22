# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc -Isrc/lib
CFLAGS += -Wno-unused-parameter

# Source and object files
SRC := $(shell find src -name '*.c')
OBJ := $(patsubst src/%.c,obj/%.o,$(SRC))

# Target binary name
TARGET = gnubg-core

# Default rule
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

# Compile each .c file into a matching .o in obj/
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf obj $(TARGET)
