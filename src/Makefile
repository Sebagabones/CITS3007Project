# Compiler and flags
CC = gcc
# Common flags - NOT for release
CFLAGS_COMMON = -std=c11 -pedantic -Wall -Wextra -Wshadow -Wconversion \
        -funsigned-char -fstack-protector-all -mshstk
# Sanitizer flags
SANITIZE_FLAGS = -fsanitize=undefined,address,leak \
        -fno-sanitize-recover=signed-integer-overflow -fno-omit-frame-pointer -fstack-protector-all
# Thread sanitizer flags
THREAD_SANITIZE_FLAGS = -fsanitize=thread -fno-omit-frame-pointer
# Comprehensive sanitizer flags (compatible with each other)
FULL_SANITIZE_FLAGS = -fsanitize=undefined,float-divide-by-zero,float-cast-overflow,bounds,null,return,vptr \
        -fno-sanitize-recover=all -fno-omit-frame-pointer
# Optimization flags
OPT_FLAGS = -O
# Optimization flags for thread sanitizer (O1 recommended)
OPT_FLAGS_THREAD = -O1
# Debug flags - disable optimization, add debug symbols, allow for gdb
DEBUG_FLAGS = -O0 -g3 -ggdb
# Full flags with sanitizers (default)
CFLAGS = $(CFLAGS_COMMON) $(SANITIZE_FLAGS) $(OPT_FLAGS) -g
# Flags without sanitizers
CFLAGS_NO_SANITIZE = $(CFLAGS_COMMON) $(OPT_FLAGS) -g
# Flags for debug builds
CFLAGS_DEBUG = $(CFLAGS_COMMON) $(DEBUG_FLAGS)
# Flags for thread safety checking
CFLAGS_THREAD = $(CFLAGS_COMMON) $(THREAD_SANITIZE_FLAGS) $(OPT_FLAGS_THREAD) -g
# Flags for comprehensive checks with compatible sanitizers
CFLAGS_FULL = $(CFLAGS_COMMON) $(FULL_SANITIZE_FLAGS) $(OPT_FLAGS_THREAD) -g

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
# Generate thread-safe executable names
THREAD_EXECS = $(SRCS:.c=.thread)
# Generate full sanitizer executable names
FULL_EXECS = $(SRCS:.c=.full)

# Default target builds all executables with sanitizers
all: $(EXECS)

# Target to build all executables without sanitizers
no-sanitize: $(NO_SANITIZE_EXECS)

# Target to build all executables with debug flags
debug: $(DEBUG_EXECS)

# Target to build all executables with thread sanitizer
thread: $(THREAD_EXECS)

# Target to build all executables with full sanitizer flags
full: $(FULL_EXECS)

# Pattern rule to build executables from object files
%: %.o
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule to build no-sanitize executables
%.no-sanitize: %.c
	$(CC) $(CFLAGS_NO_SANITIZE) -o $@ $<

# Pattern rule to build debug executables
%.debug: %.c
	$(CC) $(CFLAGS_DEBUG) -o $@ $<

# Pattern rule to build thread-safe executables
%.thread: %.c
	$(CC) $(CFLAGS_THREAD) -o $@ $<

# Pattern rule to build full sanitizer executables
%.full: %.c
	$(CC) $(CFLAGS_FULL) -o $@ $<

# Pattern rule to build object files from source files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up generated files
clean:
	rm -f $(OBJS) $(EXECS) $(NO_SANITIZE_EXECS) $(DEBUG_EXECS) $(THREAD_EXECS) $(FULL_EXECS)

# Phony targets that don't create files with these names
.PHONY: all clean no-sanitize debug thread full

# Rebuild compile_commands.json if Makefile or any .c file changes
compile-db: compile_commands.json

compile_commands.json: $(wildcard *.c) Makefile
	@echo "Cleaning and regenerating compile_commands.json..."
	@make clean
	@rm -f compile_commands.json
	@bear -- make all no-sanitize debug thread full
