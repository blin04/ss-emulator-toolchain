#include "../inc/outsection.hpp"

void OutputSection::writeWord(uint32_t location, uint32_t value) {
    if (location >= bytes.size())
        return;

    int b1 = value & 0xff;
    int b2 = (value >> 8) & 0xff;
    int b3 = (value >> 16) & 0xff;
    int b4 = (value >> 24) & 0xff;

    bytes[location] = b4;
    if (location + 1 < bytes.size())
        bytes[location + 1] = b3;
    if (location + 2 < bytes.size())
        bytes[location + 2] = b2;
    if (location + 3 < bytes.size())
        bytes[location + 3] = b1;
}