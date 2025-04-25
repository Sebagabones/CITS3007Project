
###
# Variables
# (can be overridden from command-line)
CC = gcc
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# The target executable.
# This executable is created by linking together all object files
# obtained from a .c file in the `src` directory; so exactly one
# .c file should contain a `main` function.
TARGET = $(BIN_DIR)/app

# Source code and object files
SRC_FILES := $(shell find -L $(SRC_DIR) -name "*.c")
SRC_DIRS := $(shell find -L $(SRC_DIR) -type d)
OBJ_FILES := $(SRC_FILES:.c=.o)
OBJ_FILES := $(subst $(SRC_DIR),$(BUILD_DIR),$(OBJ_FILES))
SRC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I, $(SRC_DIRS))

# Get compiler flags for installed libraries using pkg-config.
PKG_DEPS := $(shell cat libraries.txt | grep -v '^#' | xargs)
# Set PKG_CFLAGS to empty if no dependencies are found, otherwise
# use pkg-config to get the compiler flags for the dependencies
PKG_CFLAGS := $(if $(strip $(PKG_DEPS)),$(shell pkg-config --cflags $(PKG_DEPS)))
# Set PKG_LDFLAGS to empty if no dependencies are found, otherwise
# use pkg-config to get the linker flags for the dependencies
PKG_LDFLAGS := $(if $(strip $(PKG_DEPS)),$(shell pkg-config --libs $(PKG_DEPS)))

# Common flags for all builds
CFLAGS_COMMON = -std=c11 -pedantic-errors -Wall -Wextra -Wshadow -Wconversion \
        -funsigned-char -fstack-protector-all -mshstk $(INC_FLAGS) $(PKG_CFLAGS)

# Sanitizer flags
SANITIZE_FLAGS = -fsanitize=undefined,address,leak \
        -fno-sanitize-recover=signed-integer-overflow -fno-omit-frame-pointer

# Thread sanitizer flags
THREAD_SANITIZE_FLAGS = -fsanitize=thread -fno-omit-frame-pointer

# Comprehensive sanitizer flags (compatible with each other)
FULL_SANITIZE_FLAGS = -fsanitize=undefined,float-divide-by-zero,float-cast-overflow,bounds,null,return,vptr \
        -fno-sanitize-recover=all -fno-omit-frame-pointer

# Optimization flags
OPT_FLAGS = -O
OPT_FLAGS_THREAD = -O1
DEBUG_FLAGS = -g3 -ggdb -O0

# Different build configuration flags
CFLAGS = $(CFLAGS_COMMON) $(SANITIZE_FLAGS) $(OPT_FLAGS) -g
CFLAGS_NO_SANITIZE = $(CFLAGS_COMMON) $(OPT_FLAGS) -g
CFLAGS_DEBUG = $(CFLAGS_COMMON) $(DEBUG_FLAGS)
CFLAGS_THREAD = $(CFLAGS_COMMON) $(THREAD_SANITIZE_FLAGS) $(OPT_FLAGS_THREAD) -g
CFLAGS_FULL = $(CFLAGS_COMMON) $(FULL_SANITIZE_FLAGS) $(OPT_FLAGS_THREAD) -g

# Linker flags
LDFLAGS = $(PKG_LDFLAGS)
LDFLAGS_SANITIZE = $(LDFLAGS) $(SANITIZE_FLAGS)
LDFLAGS_THREAD = $(LDFLAGS) $(THREAD_SANITIZE_FLAGS)
LDFLAGS_FULL = $(LDFLAGS) $(FULL_SANITIZE_FLAGS)

# Define different target executables for each build configuration
TARGET_NO_SANITIZE = $(BIN_DIR)/app.no-sanitize
TARGET_DEBUG = $(BIN_DIR)/app.debug
TARGET_THREAD = $(BIN_DIR)/app.thread
TARGET_FULL = $(BIN_DIR)/app.full

# Define different object directories for each build configuration
OBJ_FILES_NO_SANITIZE := $(subst $(BUILD_DIR),$(BUILD_DIR)/no-sanitize,$(OBJ_FILES))
OBJ_FILES_DEBUG := $(subst $(BUILD_DIR),$(BUILD_DIR)/debug,$(OBJ_FILES))
OBJ_FILES_THREAD := $(subst $(BUILD_DIR),$(BUILD_DIR)/thread,$(OBJ_FILES))
OBJ_FILES_FULL := $(subst $(BUILD_DIR),$(BUILD_DIR)/full,$(OBJ_FILES))

###
# Targets
all: $(TARGET)

# Additional build configuration targets
no-sanitize: $(TARGET_NO_SANITIZE)
debug: $(TARGET_DEBUG)
thread: $(TARGET_THREAD)
full: $(TARGET_FULL)

# Link executables
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS_SANITIZE)

$(TARGET_NO_SANITIZE): $(OBJ_FILES_NO_SANITIZE)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_NO_SANITIZE) $(OBJ_FILES_NO_SANITIZE) -o $(TARGET_NO_SANITIZE) $(LDFLAGS)

$(TARGET_DEBUG): $(OBJ_FILES_DEBUG)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_DEBUG) $(OBJ_FILES_DEBUG) -o $(TARGET_DEBUG) $(LDFLAGS)

$(TARGET_THREAD): $(OBJ_FILES_THREAD)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_THREAD) $(OBJ_FILES_THREAD) -o $(TARGET_THREAD) $(LDFLAGS_THREAD)

$(TARGET_FULL): $(OBJ_FILES_FULL)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_FULL) $(OBJ_FILES_FULL) -o $(TARGET_FULL) $(LDFLAGS_FULL)

# Compile source files with different flags
# Default build (with sanitizers)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# No sanitizer build
$(BUILD_DIR)/no-sanitize/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_NO_SANITIZE) -MMD -MP -c $< -o $@

# Debug build
$(BUILD_DIR)/debug/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

# Thread sanitizer build
$(BUILD_DIR)/thread/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_THREAD) -MMD -MP -c $< -o $@

# Full sanitizer build
$(BUILD_DIR)/full/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_FULL) -MMD -MP -c $< -o $@

# Install dependencies
install-dependencies:
	cat apt-packages.txt | sudo ./scripts/install-deps.sh

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Generate compile database
compile-db: compile_commands.json

compile_commands.json: $(SRC_FILES) Makefile
	@echo "Cleaning and regenerating compile_commands.json..."
	@rm -f compile_commands.json
	@bear -- make all no-sanitize debug thread full

.PHONY: all clean no-sanitize debug thread full compile-db
.DELETE_ON_ERROR:

# Include automatically generated dependency files (.d)
-include $(OBJ_FILES:.o=.d)
-include $(OBJ_FILES_NO_SANITIZE:.o=.d)
-include $(OBJ_FILES_DEBUG:.o=.d)
-include $(OBJ_FILES_THREAD:.o=.d)
-include $(OBJ_FILES_FULL:.o=.d)
