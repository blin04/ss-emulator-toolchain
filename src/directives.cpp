#include "../inc/directives.hpp"

AsciiDirective::AsciiDirective(const char* str) : str(str) {}

std::vector<uint8_t> AsciiDirective::generateBytes() {
    std::vector<uint8_t> bytes;
    for (int i = 1; i < str.size() - 1; i++)    // skip " and "
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
    // for each initializer a space of 4B is allocated
    // all values are stored in little endian format
    std::vector<uint8_t> bytes;
    for (int i = 0; i < symbols.size(); i++) {
        if ((symbols[i][0] - '0') < 10) {
            long x = std::stol(symbols[i]);
            long mask = 0xffff;
            for (int i = 0; i < 4; i++) {
                bytes.push_back(x & mask);
                x >>= 8;
            }
        }
        else {
            // for now
            bytes.push_back(0xaa);
            bytes.push_back(0xbb);
            bytes.push_back(0xcc);
            bytes.push_back(0xdd);
        }
    }
    return bytes;
}
