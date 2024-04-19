# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Source file
SRCS = mem.c
# Output
TARGET = mem.out

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Rule to run the program
run: $(TARGET)
	./$(TARGET) $(INPUT) $(OUTPUT)

# Rule to clean generated files
clean:
	rm -f $(TARGET)
