CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2

all: cachesim run_tests

cachesim: src/main.cpp src/cache.cpp src/trace.cpp src/cache.hpp src/trace.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp src/cache.cpp src/trace.cpp

run_tests: tests/test_cache.cpp src/cache.cpp src/trace.cpp src/cache.hpp src/trace.hpp
	$(CXX) $(CXXFLAGS) -Isrc -o $@ tests/test_cache.cpp src/cache.cpp src/trace.cpp

test: run_tests
	./run_tests

clean:
	rm -f cachesim run_tests

.PHONY: all test clean
