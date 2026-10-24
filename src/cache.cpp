#include "cache.hpp"

namespace cachesim {

namespace {
bool isPowerOfTwo(std::uint64_t x) { return x != 0 && (x & (x - 1)) == 0; }
}  // namespace

std::string validateConfig(const CacheConfig& cfg) {
    if (cfg.cacheSizeBytes == 0) return "cache size must be greater than 0";
    if (cfg.blockSizeBytes == 0) return "block size must be greater than 0";
    if (cfg.ways == 0) return "associativity (ways) must be at least 1";
    if (!isPowerOfTwo(cfg.blockSizeBytes)) return "block size must be a power of two";
    if (cfg.cacheSizeBytes % cfg.blockSizeBytes != 0)
        return "cache size must be a multiple of the block size";

    const std::uint64_t numBlocks = cfg.cacheSizeBytes / cfg.blockSizeBytes;
    if (cfg.ways > numBlocks)
        return "associativity (" + std::to_string(cfg.ways) +
               ") cannot exceed the number of blocks in the cache (" +
               std::to_string(numBlocks) + ")";
    if (numBlocks % cfg.ways != 0)
        return "number of blocks (" + std::to_string(numBlocks) +
               ") must be divisible by associativity (" + std::to_string(cfg.ways) + ")";
    if (!isPowerOfTwo(numBlocks / cfg.ways))
        return "number of sets (" + std::to_string(numBlocks / cfg.ways) +
               ") must be a power of two";
    return "";
}

Cache::Cache(const CacheConfig& cfg)
    : cfg_(cfg),
      numSets_(cfg.cacheSizeBytes / cfg.blockSizeBytes / cfg.ways),
      lines_(numSets_ * cfg.ways) {}

AccessResult Cache::access(std::uint64_t address) {
    ++clock_;

    AccessResult r;
    r.address = address;
    r.block = address / cfg_.blockSizeBytes;  // block number in main memory
    r.set = r.block % numSets_;               // which set this block must live in
    r.tag = r.block / numSets_;               // what's left over identifies it in the set

    Line* set = &lines_[r.set * cfg_.ways];   // first line of our set

    ++stats_.accesses;

    // 1) Look for a hit: a valid line in this set with a matching tag.
    for (std::uint64_t w = 0; w < cfg_.ways; ++w) {
        if (set[w].valid && set[w].tag == r.tag) {
            set[w].lastUsed = clock_;  // refresh LRU information
            r.hit = true;
            ++stats_.hits;
            return r;
        }
    }

    // 2) Miss. Prefer an empty (invalid) line.
    ++stats_.misses;
    Line* victim = nullptr;
    for (std::uint64_t w = 0; w < cfg_.ways; ++w) {
        if (!set[w].valid) { victim = &set[w]; break; }
    }

    // 3) Set is full: evict the least recently used line.
    //    With 1 way there is only one candidate, so there is no real choice.
    if (victim == nullptr) {
        victim = &set[0];
        for (std::uint64_t w = 1; w < cfg_.ways; ++w) {
            if (set[w].lastUsed < victim->lastUsed) victim = &set[w];
        }
        r.evicted = true;
        ++stats_.evictions;
    }

    victim->valid = true;
    victim->tag = r.tag;
    victim->lastUsed = clock_;
    return r;
}

}  // namespace cachesim
