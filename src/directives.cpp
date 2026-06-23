#include "../inc/directives.hpp"
#include "../inc/objfile.hpp"
#include "../inc/symtab.hpp"

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
    long value;
    for (int i = 0; i < symbols.size(); i++) {
        if ((symbols[i][0] - '0') < 10) 
            value = std::stol(symbols[i]);
        else 
            value = (long)ObjectFile::getSymbolTable()->getSymbolValue(symbols[i]);

        long mask = 0xffff;
        for (int i = 0; i < 4; i++) {
            bytes.push_back(value & mask);
            value >>= 8;
        }
    }
    return bytes;
}
