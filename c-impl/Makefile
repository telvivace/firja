# Compiler and flags
CC = gcc
CFLAGS = -std=c2x -Wall -g
LDFLAGS = -lm
# Executable and source files
TARGET = main
SRC = main.c cpu_update.c hit_detect.c objtree.c tree_alloc.c
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
#clean:
#	rm -f $(OBJ) $(TARGET)

#.PHONY: ./main
