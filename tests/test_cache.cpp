// A tiny hand-written test runner (no external frameworks).
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "cache.hpp"
#include "trace.hpp"

using namespace cachesim;

namespace {

int failures = 0;
int checks = 0;

void check(bool condition, const std::string& message) {
    ++checks;
    if (!condition) {
        ++failures;
        std::cout << "  FAIL: " << message << "\n";
    }
}

// Runs a list of addresses through a fresh cache and checks hits/misses/evictions.
void runCase(const std::string& name, std::uint64_t size, std::uint64_t block,
             std::uint64_t ways, const std::vector<std::uint64_t>& addrs,
             std::uint64_t wantHits, std::uint64_t wantMisses, std::uint64_t wantEvictions) {
    CacheConfig cfg{size, block, ways};
    check(validateConfig(cfg).empty(), name + ": config should be valid");
    Cache cache(cfg);
    for (auto a : addrs) cache.access(a);
    const Stats& s = cache.stats();
    std::cout << name << ": hits=" << s.hits << " misses=" << s.misses
              << " evictions=" << s.evictions << "\n";
    check(s.accesses == addrs.size(), name + ": accesses");
    check(s.hits == wantHits, name + ": hits");
    check(s.misses == wantMisses, name + ": misses");
    check(s.evictions == wantEvictions, name + ": evictions");
}

void testSimulation() {
    std::cout << "[simulation]\n";
    runCase("Test 1 (16B,4B,1-way; 0,1,4,0)",   16, 4, 1, {0, 1, 4, 0},       2, 2, 0);
    runCase("Test 2 (8B,4B,1-way; 0,8,0)",       8, 4, 1, {0, 8, 0},          0, 3, 2);
    runCase("Test 3 (8B,4B,2-way; 0,8,0)",       8, 4, 2, {0, 8, 0},          1, 2, 0);
    runCase("Test 4 (8B,4B,2-way; 0,4,0,8,4)",   8, 4, 2, {0, 4, 0, 8, 4},    1, 4, 2);
}

void testMapping() {
    std::cout << "[mapping]\n";
    // 16B cache, 4B blocks, 1 way -> 4 sets. Address 0x25 = 37 -> block 9 -> set 1, tag 2.
    Cache cache(CacheConfig{16, 4, 1});
    AccessResult r = cache.access(0x25);
    check(r.block == 9, "block of 0x25 should be 9");
    check(r.set == 1, "set of 0x25 should be 1");
    check(r.tag == 2, "tag of 0x25 should be 2");
    check(!r.hit, "first access should miss");
}

void testLru() {
    std::cout << "[LRU]\n";
    // 1 set, 2 ways (8B cache, 4B blocks). Blocks A=0, B=4, C=8 all map to the same set.
    Cache cache(CacheConfig{8, 4, 2});
    cache.access(0);   // A miss
    cache.access(4);   // B miss
    cache.access(0);   // A hit -> B is now least recently used
    AccessResult r = cache.access(8);  // C miss, must evict B (not A)
    check(r.evicted, "C should evict a line");
    check(cache.access(0).hit, "A should still be cached (B was the LRU victim)");
    check(!cache.access(4).hit, "B should have been evicted");
}

void testValidation() {
    std::cout << "[validation]\n";
    check(!validateConfig(CacheConfig{0, 4, 1}).empty(), "size 0 rejected");
    check(!validateConfig(CacheConfig{16, 0, 1}).empty(), "block 0 rejected");
    check(!validateConfig(CacheConfig{16, 4, 0}).empty(), "ways 0 rejected");
    check(!validateConfig(CacheConfig{24, 3, 1}).empty(), "block not power of 2 rejected");
    check(!validateConfig(CacheConfig{18, 4, 1}).empty(), "size not multiple of block rejected");
    check(!validateConfig(CacheConfig{8, 4, 4}).empty(), "ways > blocks rejected");
    check(!validateConfig(CacheConfig{24, 4, 2}).empty(), "blocks not divisible by ways rejected");
    check(!validateConfig(CacheConfig{24, 4, 1}).empty(), "non power-of-2 set count rejected");
    check(validateConfig(CacheConfig{16, 4, 4}).empty(), "fully associative accepted");
}

void testTraceParsing() {
    std::cout << "[trace parsing]\n";
    std::uint64_t v = 0;
    check(parseHexAddress("1A", v) && v == 26, "plain hex");
    check(parseHexAddress("0xff", v) && v == 255, "0x prefix, lowercase");
    check(parseHexAddress("0XFF", v) && v == 255, "0X prefix");
    check(!parseHexAddress("", v), "empty rejected");
    check(!parseHexAddress("0x", v), "bare prefix rejected");
    check(!parseHexAddress("12G", v), "bad digit rejected");
    check(!parseHexAddress("-1", v), "negative rejected");
    check(!parseHexAddress("11111111111111111", v), "more than 64 bits rejected");

    std::istringstream good("# comment\n\n  0x10  \n20\n   # indented comment\n");
    std::vector<std::uint64_t> addrs;
    std::string err;
    check(readTrace(good, addrs, err), "good trace parses");
    check(addrs.size() == 2 && addrs[0] == 16 && addrs[1] == 32, "comments/blank lines skipped");

    std::istringstream bad("10\nzz\n");
    addrs.clear();
    check(!readTrace(bad, addrs, err), "bad trace fails");
    check(err.find("line 2") != std::string::npos, "error names line 2");
}

}  // namespace

int main() {
    testSimulation();
    testMapping();
    testLru();
    testValidation();
    testTraceParsing();
    std::cout << "\n" << (checks - failures) << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
