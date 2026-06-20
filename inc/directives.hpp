#ifndef _DIRECTIVES_H_
#define _DIRECTIVES_H_

#include "line.hpp"

class SkipDirective : Line {
public:
    SkipDirective(int bytesCount);
    std::vector<uint8_t> generateBytes();
private:
    int count;
};

#endif