#ifndef _LINE_H_
#define _LINE_H_

#include <cstdint>
#include <vector>

class Line {
public:
    virtual std::vector<uint8_t> generateBytes() = 0; 
};

#endif