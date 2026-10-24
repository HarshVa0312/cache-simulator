// trace.hpp - reading a list of byte addresses from text.
#pragma once

#include <cstdint>
#include <istream>
#include <string>
#include <vector>

namespace cachesim {

// Parses one hex address ("1A3F" or "0x1A3F"). Returns false if malformed.
bool parseHexAddress(const std::string& text, std::uint64_t& out);

// Reads addresses from a stream, one per line. Blank lines and lines starting
// with '#' are skipped. On a bad line, returns false and sets `error`
// (including the line number).
bool readTrace(std::istream& in, std::vector<std::uint64_t>& addresses, std::string& error);

}  // namespace cachesim
