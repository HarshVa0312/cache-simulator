#include "trace.hpp"

#include <cctype>

namespace cachesim {

namespace {
std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}
}  // namespace

bool parseHexAddress(const std::string& text, std::uint64_t& out) {
    std::string s = text;
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s = s.substr(2);
    if (s.empty() || s.size() > 16) return false;  // 16 hex digits = 64 bits

    std::uint64_t value = 0;
    for (char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        int digit = std::isdigit(static_cast<unsigned char>(c))
                        ? c - '0'
                        : std::tolower(static_cast<unsigned char>(c)) - 'a' + 10;
        value = value * 16 + static_cast<std::uint64_t>(digit);
    }
    out = value;
    return true;
}

bool readTrace(std::istream& in, std::vector<std::uint64_t>& addresses, std::string& error) {
    std::string line;
    std::size_t lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::uint64_t addr = 0;
        if (!parseHexAddress(line, addr)) {
            error = "line " + std::to_string(lineNo) + ": '" + line +
                    "' is not a valid hexadecimal address";
            return false;
        }
        addresses.push_back(addr);
    }
    return true;
}

}  // namespace cachesim
