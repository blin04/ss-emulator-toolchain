#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include "line.hpp"

class HaltInstruction : Line {
public:
    std::vector<uint8_t> generateBytes();
};

#endif