#ifndef _DIRECTIVES_H_
#define _DIRECTIVES_H_

#include <string>

#include "line.hpp"

class SkipDirective : public Line {
public:
    SkipDirective(int bytesCount);
    std::vector<uint8_t> generateBytes();
private:
    int count;
};

class WordDirective : public Line {
public:
    WordDirective(std::vector<std::string> symbols);
    std::vector<uint8_t> generateBytes();
private:
    std::vector<std::string> symbols;
};

class AsciiDirective : public Line {
public:
    AsciiDirective(std::string str);
    std::vector<uint8_t> generateBytes();
private:
    std::vector<std::string> str;
};

#endif