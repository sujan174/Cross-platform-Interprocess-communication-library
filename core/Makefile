# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Iinclude -Wall -Wextra

# Source files
SRCS = src/main.c src/named_pipe.c src/ordinary_pipe.c src/shared_memory.c

# Output executable
TARGET = miniipc.exe

# Build target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lkernel32 -luser32 -ladvapi32

# Clean target
clean:
	del $(TARGET)