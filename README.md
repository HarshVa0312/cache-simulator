# Cache Memory Simulator

A small trace-driven cache simulator in C++17 (standard library only). It reads a list of
byte addresses, simulates a direct-mapped or set-associative cache with LRU replacement,
and reports hits, misses, evictions, and hit/miss rates.

## What it does

- Reads hexadecimal byte addresses from a text file (one per line; blank lines and `#` lines are ignored; `0x` prefix optional).
- Lets you configure cache size (bytes), block size (bytes) and associativity (ways).
- For each address computes the memory block, cache set and tag, then reports HIT or MISS.
- Uses LRU replacement when a set is full. A direct-mapped cache (1 way) has only one possible line per set, so there is no replacement choice.
- Validates the configuration and the trace and prints clear error messages.

## Project layout

```
cache-simulator/
├── Makefile
├── README.md
├── src/        cache.hpp  cache.cpp  trace.hpp  trace.cpp  main.cpp
├── tests/      test_cache.cpp
├── traces/     sequential.txt  conflict.txt  cycle3.txt  vector_add.txt
└── scripts/    compare.sh
```

## Build and run

```
make                 # builds ./cachesim and ./run_tests
make test            # runs the tests
./cachesim --trace traces/conflict.txt --size 64 --block 16 --ways 2
./cachesim --trace traces/conflict.txt --size 64 --block 16 --ways 1 --verbose
sh scripts/compare.sh        # associativity comparison (defaults: 64 B cache, 16 B blocks)
```

Manual compile: `g++ -std=c++17 -Wall -Wextra -pedantic -o cachesim src/main.cpp src/cache.cpp src/trace.cpp`

Options: `--trace <file>`, `--size <bytes>`, `--block <bytes>`, `--ways <n>` (default 1), `--verbose`.

## Mapping formulas

```
block  = address / block_size
sets   = (cache_size / block_size) / ways
set    = block % sets
tag    = block / sets
```

A lookup hits if any line in `set` is valid and has the same `tag`.
On a miss, an empty line in the set is used if there is one; otherwise the least recently used line is evicted.

## Configuration rules

block size is a power of two; cache size is a multiple of block size; ways >= 1 and no larger than the number of blocks;
number of blocks is divisible by ways; number of sets is a power of two.

## Tests

`make test` runs 48 checks, including the four required cases:

| # | Cache / block / ways | Addresses | Expected | Result |
|---|---|---|---|---|
| 1 | 16 B / 4 B / 1 | 0, 1, 4, 0 | 2 hits, 2 misses | 2 hits, 2 misses (pass) |
| 2 | 8 B / 4 B / 1 | 0, 8, 0 | 0 hits, 3 misses | 0 hits, 3 misses (pass) |
| 3 | 8 B / 4 B / 2 | 0, 8, 0 | 1 hit, 2 misses | 1 hit, 2 misses (pass) |
| 4 | 8 B / 4 B / 2 | 0, 4, 0, 8, 4 | 1 hit, 4 misses | 1 hit, 4 misses (pass) |

It also tests the set/tag calculation, LRU victim choice, configuration validation, and trace parsing.

## Associativity comparison (64 B cache, 16 B blocks, LRU)

| trace | 1-way | 2-way | 4-way |
|---|---|---|---|
| conflict (2 addresses alternate) | 0.00% | 75.00% | 75.00% |
| cycle3 (3 addresses round-robin) | 0.00% | 0.00% | 75.00% |
| sequential (stride 8, two passes) | 75.00% | 75.00% | 75.00% |
| vector_add (c[i] = a[i] + b[i]) | 0.00% | 75.00% | 75.00% |

- **conflict / vector_add:** the addresses map to the same set in the direct-mapped cache and keep evicting each other (conflict misses). Two ways remove the problem.
- **cycle3:** three blocks compete for one set. With 2 ways, LRU always evicts the block needed next, so there are zero hits. 4 ways holds all three.
- **sequential:** the only misses are the first touch of each block, so associativity does not matter.

## Limitations

Reads only (no writes, write policies or dirty bits); single-level cache; no timing/AMAT; LRU is the only replacement policy;
addresses are treated as plain byte addresses (the trace has no read/write marker); the set count must be a power of two.
