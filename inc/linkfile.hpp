#ifndef _LINK_FILE_H_
#define _LINK_FILE_H_

#include <cstdint>
#include <string>
#include <vector>

#include "section.hpp"      // RelocType
#include "symtab.hpp"       // SymbolTable::SymbolBind

// In-memory representation of a single parsed input object file, as
// produced by ObjReader::parse(). Mirrors the on-disk layout of
// main.txt: one local symbol table plus one blob per section
// (code/data bytes followed by literal pool bytes) plus that
// section's relocation list.
//
// Indexes below (LocalSymbol::index, RelaEntry::symbolIndex) are
// scoped to THIS file only, exactly as assigned by the assembler run
// that produced it (SymbolTable::symbol_index restarts at 1 per
// run). Never compare/merge these indexes across different LinkFile
// instances directly - go through GlobalSymbolTable instead.
class LinkFile {
public:
    struct LocalSymbol {
        int                     index;
        std::string             name;
        int                     section;      // section id, or UND marker
        int                     value;
        SymbolTable::SymbolBind bind;
        bool                    defined;
    };

    struct RelaEntry {
        int         offset;
        RelocType   type;
        int         symbolIndex;   // -> LocalSymbol::index, per-file scoped
        int         addend;
    };

    struct RawSection {
        std::string             name;
        std::vector<uint8_t>    bytes;   // code/data bytes + literal pool bytes
        std::vector<RelaEntry>  relas;
    };

    std::string                sourceFilename;
    std::vector<LocalSymbol>   symbols;
    std::vector<RawSection>    sections;
};

#endif
