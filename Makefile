# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -Wall $(shell pkg-config --cflags raylib)

# Linker flags
LDFLAGS = $(shell pkg-config --libs raylib) -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGL

# Source directory
SRC_DIR = src

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(SRCS:.c=.o)

# Output executable
TARGET = funk

# Build target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile .c files to .o files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run and clean
run: $(TARGET)
	./$(TARGET)
	$(MAKE) clean

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)
