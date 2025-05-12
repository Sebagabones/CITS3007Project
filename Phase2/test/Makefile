#  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

# Uncomment the following line to generate a minimal log.
# COMPACT="-DTST_STR_COMPACT"

# Uncomment the following line to disable ANSI colors
# NOCOLOR="-DTST_STR_NOCOLOR"

# Uncomment the following line to enable debugging
# DEBUG="-DDEBUG"

_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

SRC_DIR=../ScaffoldingFilesChanged/src
TEST_DIR=.
# CXX_AVAILABLE := $(shell $(CXX) --version 2>/dev/null)

CFLAGS=-O2 -Wall -I$(SRC_DIR)/ $(STATIC) $(COMPACT) $(DEBUG) -DCITS3007_PROJECT_TESTING
LIBS=

TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)

ifneq ($(CXX_AVAILABLE),)
  TESTS_SRC_CPP=$(wildcard t_*.cpp)
  TESTS_CPP=$(TESTS_SRC_CPP:.cpp=)
	CXXFLAGS=$(CFLAGS)
endif

TESTS_SRC=$(TST_SRC_C) $(TST_SRC_CPP)
TESTS=$(TESTS_C) $(TESTS_CPP)

SRC_OBJS=$(SRC_DIR)/login.o $(SRC_DIR)/account.o $(TEST_DIR)/stubs.o $(SRC_DIR)/pwHandling.o

LIBS = -lbsd -lsodium -largon2
CFLAGS += -DHAVE_STRLCPY

# targets
all: $(TESTS)

runtest: all
	./t_login

MAKEFLAGS += --no-builtin-rules

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o $(SRC_OBJS)
	$(CC) -o $@ $^ $(LIBS)
# Specific rule for compiling test stubs
$(TEST_DIR)/stubs.o: $(TEST_DIR)/stubs.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log functions.o $(SRC_OBJS)
