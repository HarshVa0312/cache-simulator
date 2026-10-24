#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "cache.hpp"
#include "trace.hpp"

using namespace cachesim;

namespace {

void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --trace <file> --size <bytes> --block <bytes> [--ways <n>] [--verbose]\n"
              << "  --trace    text file of hex byte addresses, one per line\n"
              << "  --size     total cache size in bytes\n"
              << "  --block    block (line) size in bytes, power of two\n"
              << "  --ways     lines per set; 1 = direct-mapped (default 1)\n"
              << "  --verbose  print block/set/tag and HIT/MISS for every access\n";
}

// Parses a non-negative decimal integer; returns false on anything else.
bool parseUnsigned(const std::string& s, std::uint64_t& out) {
    if (s.empty() || s.size() > 18) return false;
    std::uint64_t v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        v = v * 10 + static_cast<std::uint64_t>(c - '0');
    }
    out = v;
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    CacheConfig cfg;
    std::string tracePath;
    bool verbose = false;
    bool haveSize = false, haveBlock = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--verbose") { verbose = true; continue; }
        if (arg == "--help" || arg == "-h") { printUsage(argv[0]); return 0; }

        if (arg == "--trace" || arg == "--size" || arg == "--block" || arg == "--ways") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " needs a value\n";
                return 1;
            }
            const std::string value = argv[++i];
            if (arg == "--trace") { tracePath = value; continue; }

            std::uint64_t n = 0;
            if (!parseUnsigned(value, n)) {
                std::cerr << "Error: " << arg << " must be a non-negative whole number, got '"
                          << value << "'\n";
                return 1;
            }
            if (arg == "--size") { cfg.cacheSizeBytes = n; haveSize = true; }
            else if (arg == "--block") { cfg.blockSizeBytes = n; haveBlock = true; }
            else { cfg.ways = n; }
            continue;
        }
        std::cerr << "Error: unknown option '" << arg << "'\n";
        printUsage(argv[0]);
        return 1;
    }

    if (tracePath.empty() || !haveSize || !haveBlock) {
        std::cerr << "Error: --trace, --size and --block are required\n";
        printUsage(argv[0]);
        return 1;
    }

    const std::string cfgError = validateConfig(cfg);
    if (!cfgError.empty()) {
        std::cerr << "Error: invalid cache configuration: " << cfgError << "\n";
        return 1;
    }

    std::ifstream file(tracePath);
    if (!file) {
        std::cerr << "Error: cannot open trace file '" << tracePath << "'\n";
        return 1;
    }
    std::vector<std::uint64_t> addresses;
    std::string traceError;
    if (!readTrace(file, addresses, traceError)) {
        std::cerr << "Error: " << tracePath << ", " << traceError << "\n";
        return 1;
    }

    Cache cache(cfg);
    if (verbose) {
        std::cout << "address      block        set     tag          result\n";
    }
    for (std::uint64_t addr : addresses) {
        const AccessResult r = cache.access(addr);
        if (verbose) {
            std::cout << "0x" << std::left << std::hex << std::setw(10) << r.address
                      << std::dec << std::setw(13) << r.block << std::setw(8) << r.set
                      << std::setw(13) << r.tag << (r.hit ? "HIT" : "MISS")
                      << (r.evicted ? " (evicted a line)" : "") << "\n";
        }
    }

    const Stats& s = cache.stats();
    const char* kind = cfg.ways == 1 ? "direct-mapped" : "set-associative";
    std::cout << "Cache: " << cfg.cacheSizeBytes << " B, block " << cfg.blockSizeBytes << " B, "
              << cfg.ways << "-way (" << kind << "), " << cache.numSets() << " sets\n"
              << "Trace: " << tracePath << "\n"
              << "Total accesses: " << s.accesses << "\n"
              << "Hits:           " << s.hits << "\n"
              << "Misses:         " << s.misses << "\n"
              << "Evictions:      " << s.evictions << "\n"
              << std::fixed << std::setprecision(2)
              << "Hit rate:       " << s.hitRate() * 100 << "%\n"
              << "Miss rate:      " << s.missRate() * 100 << "%\n";
    return 0;
}
