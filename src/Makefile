###
# Variables
CC = gcc
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

TARGET = $(BIN_DIR)/app

TS_FILES := $(shell find -L $(SRC_DIR) -name "*.ts")
C_GEN_SRC := $(TS_FILES:.ts=.c)

SRC_FILES := $(shell find -L $(SRC_DIR) -name "*.c") $(C_GEN_SRC)
SRC_DIRS := $(shell find -L $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I, $(SRC_DIRS))

# Deduplicate .o files
OBJ_FILES := $(SRC_FILES:.c=.o)
OBJ_FILES := $(subst $(SRC_DIR),$(BUILD_DIR),$(OBJ_FILES))
OBJ_FILES := $(shell echo $(OBJ_FILES) | tr ' ' '\n' | sort | uniq)

PKG_DEPS := $(shell cat libraries.txt | grep -v '^#' | xargs)
PKG_CFLAGS := $(if $(strip $(PKG_DEPS)),$(shell pkg-config --cflags $(PKG_DEPS)))
PKG_LDFLAGS := $(if $(strip $(PKG_DEPS)),$(shell pkg-config --libs $(PKG_DEPS)))

CFLAGS_COMMON = -std=c11 -pedantic-errors -Wall -Wextra -Wshadow -Wconversion \
        -funsigned-char -fstack-protector-all -mshstk $(INC_FLAGS) $(PKG_CFLAGS)

SANITIZE_FLAGS = -fsanitize=undefined,address,leak \
        -fno-sanitize-recover=signed-integer-overflow -fno-omit-frame-pointer
THREAD_SANITIZE_FLAGS = -fsanitize=thread -fno-omit-frame-pointer
FULL_SANITIZE_FLAGS = -fsanitize=undefined,float-divide-by-zero,float-cast-overflow,bounds,null,return,vptr \
        -fno-sanitize-recover=all -fno-omit-frame-pointer

OPT_FLAGS = -O
OPT_FLAGS_THREAD = -O1
DEBUG_FLAGS = -g3 -ggdb -O0

CFLAGS = $(CFLAGS_COMMON) $(SANITIZE_FLAGS) $(OPT_FLAGS) -g
CFLAGS_NO_SANITIZE = $(CFLAGS_COMMON) $(OPT_FLAGS) -g
CFLAGS_DEBUG = $(CFLAGS_COMMON) $(DEBUG_FLAGS)
CFLAGS_THREAD = $(CFLAGS_COMMON) $(THREAD_SANITIZE_FLAGS) $(OPT_FLAGS_THREAD) -g
CFLAGS_FULL = $(CFLAGS_COMMON) $(FULL_SANITIZE_FLAGS) $(OPT_FLAGS_THREAD) -g

LDFLAGS = $(PKG_LDFLAGS)
LDFLAGS_SANITIZE = $(LDFLAGS) $(SANITIZE_FLAGS)
LDFLAGS_THREAD = $(LDFLAGS) $(THREAD_SANITIZE_FLAGS)
LDFLAGS_FULL = $(LDFLAGS) $(FULL_SANITIZE_FLAGS)

TARGET_NO_SANITIZE = $(BIN_DIR)/app.no-sanitize
TARGET_DEBUG = $(BIN_DIR)/app.debug
TARGET_THREAD = $(BIN_DIR)/app.thread
TARGET_FULL = $(BIN_DIR)/app.full

OBJ_FILES_NO_SANITIZE := $(subst $(BUILD_DIR),$(BUILD_DIR)/no-sanitize,$(OBJ_FILES))
OBJ_FILES_DEBUG := $(subst $(BUILD_DIR),$(BUILD_DIR)/debug,$(OBJ_FILES))
OBJ_FILES_THREAD := $(subst $(BUILD_DIR),$(BUILD_DIR)/thread,$(OBJ_FILES))
OBJ_FILES_FULL := $(subst $(BUILD_DIR),$(BUILD_DIR)/full,$(OBJ_FILES))

###
# Targets

all: $(TARGET)

no-sanitize: $(TARGET_NO_SANITIZE)
debug: $(TARGET_DEBUG)
thread: $(TARGET_THREAD)
full: $(TARGET_FULL)

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

# Rule for .c generated from .ts
%.c: %.ts
	checkmk $< > $@

# Compile C source with various configurations (adds banned header)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	./add_banned_header.pl $<
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/no-sanitize/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	./add_banned_header.pl $<
	$(CC) $(CFLAGS_NO_SANITIZE) -MMD -MP -c $< -o $@

$(BUILD_DIR)/debug/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	./add_banned_header.pl $<
	$(CC) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

$(BUILD_DIR)/thread/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	./add_banned_header.pl $<
	$(CC) $(CFLAGS_THREAD) -MMD -MP -c $< -o $@

$(BUILD_DIR)/full/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	./add_banned_header.pl $<
	$(CC) $(CFLAGS_FULL) -MMD -MP -c $< -o $@

install-dependencies:
	cat apt-packages.txt | sudo ./scripts/install-deps.sh

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) src/check*.c src/*.BAK src/*.NEW

compile-db: compile_commands.json

compile_commands.json: $(SRC_FILES) Makefile
	@echo "Cleaning and regenerating compile_commands.json..."
	@rm -f compile_commands.json
	@bear -- make all no-sanitize debug thread full

.PHONY: all clean no-sanitize debug thread full install-dependencies compile-db
.DELETE_ON_ERROR:

-include $(OBJ_FILES:.o=.d)
-include $(OBJ_FILES_NO_SANITIZE:.o=.d)
-include $(OBJ_FILES_DEBUG:.o=.d)
-include $(OBJ_FILES_THREAD:.o=.d)
-include $(OBJ_FILES_FULL:.o=.d)
