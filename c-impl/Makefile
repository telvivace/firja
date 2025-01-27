# Compiler and flags
CC = gcc
CFLAGS = -std=c2x -Wall -g 
LDFLAGS = -lm -lSDL2
# Executable and source files
TARGET = main
SRC =  cpu_update.c hit_detect.c objtree.c tree_alloc.c treeutils.c
MAINSRC = main.c
MAINOBJ = $(MAINSRC:.c=.o)
OBJ = $(SRC:.c=.o)

# Tester
TESTS = tests
TESTSRC = tests.c
TESTOBJ = $(TESTSRC:.c=.o)
# Default target
all: $(TARGET)

test: CFLAGS += -fsanitize=address,undefined,leak
test: $(TESTS) 

profile: CFLAGS = -std=c2x -O3 -fprofile-generate 
profile: $(TARGET)
	rm *.gcda
debug: CFLAGS += -fsanitize=address,undefined,leak
debug: $(TARGET)

release: CFLAGS = -std=c2x -O3 -fprofile-use -fprofile-correction
release: $(TARGET)
# Link test file with object files
$(TESTS): $(OBJ) $(TESTSRC)
	$(CC) $(CFLAGS) -o $(TESTS) $(TESTSRC) $(OBJ) $(LDFLAGS)

# Link object files to create the executable
$(TARGET): $(OBJ) $(SRC) $(MAINSRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(MAINSRC) $(OBJ) $(LDFLAGS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ) $(TARGET)

#.PHONY: ./main
