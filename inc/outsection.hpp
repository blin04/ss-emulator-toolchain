#ifndef _LINK_SECTION_H_
#define _LINK_SECTION_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// One output section, built by concatenating every input file's
// contribution (that file's own section bytes + its own literal
// pool bytes, in file order). fileOffsets records where each
// contributing file's blob starts within `bytes`, which both linker
// modes need: to translate that file's relocation offsets, and to
// translate its section-relative symbol values into merged-section-
// relative terms.
//
// baseAddress is only meaningful in full-link mode, filled in by the
// placement pass; relocatable mode never assigns it.
class OutputSection {
public:
    std::string             name;
    uint32_t                baseAddress = 0;
    std::vector<uint8_t>    bytes;
    std::map<int, int>      fileOffsets;   // fileIndex -> offset in bytes within the section
    std::vector<LinkFile::RelocEntry> relas;

    // note: can fail silently
    inline void writeWord(uint32_t location, uint32_t value) {
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
};

#endif
