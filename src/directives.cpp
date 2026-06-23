#include "../inc/directives.hpp"

AsciiDirective::AsciiDirective(const char* str) : str(str) {}

std::vector<uint8_t> AsciiDirective::generateBytes() {
    std::vector<uint8_t> bytes;
    for (int i = 0; i < str.size(); i++)
        bytes.push_back(static_cast<uint8_t>(str[i]));
    return bytes;
}

SkipDirective::SkipDirective(int bytesCount) : count(bytesCount) {}

std::vector<uint8_t> SkipDirective::generateBytes() {
    std::vector<uint8_t> bytes(count, 0);
    return bytes;
}

WordDirective::WordDirective(std::vector<std::string> symbols) : symbols(symbols) {}

std::vector<uint8_t> WordDirective::generateBytes() {
    std::vector<uint8_t> bytes;
    for (int i = 0; i < symbols.size(); i++) {
        if ((symbols[i][0] - '0') < 10) {
            // literals are stored in little endian format
            long x = std::stol(symbols[i]);
            long mask = 0xffff;
            while (x) {
                bytes.push_back(x & mask);
                x >>= 8;
            }
        }
        else {
            // for now
            bytes.push_back(0xab);
            bytes.push_back(0xba);
        }
    }
    return bytes;
}
