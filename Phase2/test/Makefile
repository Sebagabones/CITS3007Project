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

# Sanitizer flags - used both in compilation and linking

SANITIZE_FLAGS_MORE=-fsanitize=undefined,float-divide-by-zero,float-cast-overflow,bounds,null,return,vptr -fno-sanitize-recover=all -fno-omit-frame-pointer -fsanitize=undefined,address,leak -fno-sanitize-recover=signed-integer-overflow -fno-omit-frame-pointer

# valgrind tests are commented out, uncomment for testing
CFLAGS=-O3 -Wall -std=c11 -funsigned-char -fstack-protector-all -mshstk $(SANITIZE_FLAGS_MORE) -I$(TEST_DIR)/ -I$(SRC_DIR)/ $(STATIC) $(COMPACT) $(DEBUG) -isystem /usr/include/bsd -DLIBBSD_OVERLAY -DCITS3007_PROJECT_TESTING
# CFLAGS= -g -O0 -Wall -std=c11 -funsigned-char -fstack-protector-all -mshstk -I$(TEST_DIR)/ -I$(SRC_DIR)/ $(STATIC) $(COMPACT) $(DEBUG) -isystem /usr/include/bsd -DLIBBSD_OVERLAY -DCITS3007_PROJECT_TESTING  #valgrind


TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)
ifneq ($(CXX_AVAILABLE),)
  TESTS_SRC_CPP=$(wildcard t_*.cpp)
  TESTS_CPP=$(TESTS_SRC_CPP:.cpp=)
  CXXFLAGS=$(CFLAGS)
endif
TESTS_SRC=$(TST_SRC_C) $(TST_SRC_CPP)
TESTS=$(TESTS_C) $(TESTS_CPP)
SRC_OBJS=$(SRC_DIR)/login.o $(SRC_DIR)/account.o $(TEST_DIR)/stubs.o
# LIBS = -lbsd -lsodium -largon2 #valgrind
LIBS = -fsanitize=address -lbsd -lsodium -largon2
#LDFLAGS = -lbsd -lsodium -largon2 #valgrind
LDFLAGS = $(SANITIZE_FLAGS_MORE) -lbsd -lsodium -largon2

# targets
all: $(TESTS)
runtest: all
	./tstrun --color
MAKEFLAGS += --no-builtin-rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o $(SRC_OBJS)
	#$(CC) -o $@ $^ $(LIBS) #valgrind
	$(CC) $(SANITIZE_FLAGS_MORE) -o $@ $^ $(LIBS)

# Specific rule for compiling test stubs
$(TEST_DIR)/stubs.o: $(TEST_DIR)/stubs.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: %.o
clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log functions.o $(SRC_OBJS)
