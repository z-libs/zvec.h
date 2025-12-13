
BUNDLER = z-core/zbundler.py
SRC = src/zvec.c
DIST = zvec.h

CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c11 -O2
CXXFLAGS = -Wall -Wextra -std=c++11 -O2

all: bundle

bundle:
	@echo "Bundling $(DIST)..."
	python3 $(BUNDLER) $(SRC) $(DIST)

init:
	git submodule update --init --recursive


test: bundle test_c test_cpp

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

.PHONY: all bundle init test test_c test_cpp
