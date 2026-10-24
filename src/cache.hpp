// cache.hpp - the cache model: configuration, statistics, and the Cache class.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cachesim {

// The three numbers the user chooses.
struct CacheConfig {
    std::uint64_t cacheSizeBytes = 0;  // total capacity of the cache
    std::uint64_t blockSizeBytes = 0;  // size of one cache line ("block")
    std::uint64_t ways = 1;            // lines per set (1 = direct-mapped)
};

// Returns an empty string if the config is valid, otherwise a clear error message.
std::string validateConfig(const CacheConfig& cfg);

// Everything we learn from a single memory access.
struct AccessResult {
    std::uint64_t address = 0;
    std::uint64_t block = 0;  // which block of memory the address belongs to
    std::uint64_t set = 0;    // which cache set that block maps to
    std::uint64_t tag = 0;    // identifies the block within its set
    bool hit = false;
    bool evicted = false;     // true if a valid line had to be thrown out
};

struct Stats {
    std::uint64_t accesses = 0;
    std::uint64_t hits = 0;
    std::uint64_t misses = 0;
    std::uint64_t evictions = 0;

    double hitRate() const { return accesses ? static_cast<double>(hits) / accesses : 0.0; }
    double missRate() const { return accesses ? static_cast<double>(misses) / accesses : 0.0; }
};

class Cache {
public:
    // Precondition: validateConfig(cfg) returned "" (the constructor does not re-check).
    explicit Cache(const CacheConfig& cfg);

    AccessResult access(std::uint64_t address);

    const Stats& stats() const { return stats_; }
    std::uint64_t numSets() const { return numSets_; }

private:
    struct Line {
        bool valid = false;
        std::uint64_t tag = 0;
        std::uint64_t lastUsed = 0;  // "time" of most recent use, for LRU
    };

    CacheConfig cfg_;
    std::uint64_t numSets_ = 0;
    std::uint64_t clock_ = 0;        // increases by 1 on every access
    std::vector<Line> lines_;        // numSets_ * ways lines; set s starts at s * ways
    Stats stats_;
};

}  // namespace cachesim
