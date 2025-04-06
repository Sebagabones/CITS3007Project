# Compiler and flags
CC = gcc

# Common flags
CFLAGS_COMMON = -std=c11 -pedantic -Wall -Wextra -Wshadow -Wconversion \
        -funsigned-char

# Sanitizer flags
SANITIZE_FLAGS = -fsanitize=undefined,address,leak \
        -fno-sanitize-recover=signed-integer-overflow -fno-omit-frame-pointer

# Optimization flags
OPT_FLAGS = -O

# Debug flags - disable optimization, add debug symbols, allow for gdb
DEBUG_FLAGS = -O0 -g3 -ggdb

# Full flags with sanitizers (default)
CFLAGS = $(CFLAGS_COMMON) $(SANITIZE_FLAGS) $(OPT_FLAGS) -g

# Flags without sanitizers
CFLAGS_NO_SANITIZE = $(CFLAGS_COMMON) $(OPT_FLAGS) -g

# Flags for debug builds
CFLAGS_DEBUG = $(CFLAGS_COMMON) $(DEBUG_FLAGS)

# Get all .c files in the current directory
SRCS = $(wildcard *.c)
# Generate object file names from source files
OBJS = $(SRCS:.c=.o)
# Generate executable names from source files (without extension)
EXECS = $(SRCS:.c=)
# Generate non-sanitized executable names
NO_SANITIZE_EXECS = $(SRCS:.c=.no-sanitize)
# Generate debug executable names
DEBUG_EXECS = $(SRCS:.c=.debug)

# Default target builds all executables with sanitizers
all: $(EXECS)

# Target to build all executables without sanitizers
no-sanitize: $(NO_SANITIZE_EXECS)

# Target to build all executables with debug flags
debug: $(DEBUG_EXECS)

# Pattern rule to build executables from object files
%: %.o
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule to build no-sanitize executables
%.no-sanitize: %.c
	$(CC) $(CFLAGS_NO_SANITIZE) -o $@ $<

# Pattern rule to build debug executables
%.debug: %.c
	$(CC) $(CFLAGS_DEBUG) -o $@ $<

# Pattern rule to build object files from source files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(OBJS) $(EXECS) $(NO_SANITIZE_EXECS) $(DEBUG_EXECS)

# Phony targets that don't create files with these names
.PHONY: all clean no-sanitize debug
