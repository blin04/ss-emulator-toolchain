#ifndef _HEX_READER_H_
#define _HEX_READER_H_

#include <string>

#include "cpu.hpp"

// Parses the flat memory dump produced by the linker's -hex mode
// (`ADDR: b1 b2 ... b8` per line, hex throughout, two words per line)
// and loads every resulting 32-bit word into the CPU via loadWord().
class HexReader {
public:
    static void load(const std::string& path, CPU& cpu);
};

#endif
