# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -Wall $(shell pkg-config --cflags raylib)

# Linker flags
LDFLAGS = $(shell pkg-config --libs raylib) -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGL

# Source files
SRCS = main.c

# Output executable
TARGET = main

# Build target
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Run
run:
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
	./main
	rm -f $(TARGET)
	rm -f txt

# Clean target
clean:
	rm -f $(TARGET)
	rm -f txt
