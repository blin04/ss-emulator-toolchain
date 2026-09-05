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
};

#endif
