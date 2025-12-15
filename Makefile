
BUNDLER = z-core/zbundler.py
SRC = src/zvec.c
DIST = zvec.h

CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -O2 -I.
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -I.

all: bundle get_zerror_h

bundle:
	@echo "Bundling $(DIST)..."
	python3 $(BUNDLER) $(SRC) $(DIST)

get_zerror_h:
	@echo "Using wget to add 'zerror.h'..."
	wget -q "https://raw.githubusercontent.com/z-libs/zerror.h/main/zerror.h" -O "zerror.h"

clean:
	@echo "Removing 'zerror.h'..."
	@rm zerror.h

init:
	git submodule update --init --recursive


test: bundle test_c test_cpp clean

test_c:
	@echo "----------------------------------------"
	@echo "Building C Tests..."
	@$(CC) $(CFLAGS) tests/test_main.c -o tests/runner_c
	@./tests/runner_c
	@rm tests/runner_c

test_cpp:
	@echo "----------------------------------------"
	@echo "Building C++ Tests..."
	@$(CXX) $(CXXFLAGS) tests/test_cpp.cpp -o tests/runner_cpp
	@./tests/runner_cpp
	@rm tests/runner_cpp

.PHONY: all bundle get_zerror_h init test test_c test_cpp clean
