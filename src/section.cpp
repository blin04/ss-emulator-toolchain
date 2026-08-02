#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

#include <iostream>

// sections are numbered starting from 1
int Section::counter = 1;

Section::Section(std::string sectionName, int offset) 
    : index(counter++) 
    , name(sectionName)
    , offset(offset)
    , startAddress(0) {
    std::cout << "SectionTable[" << index << "]: created " << name << " at " << offset << "B from the start\n";
    ObjectFile::getSymbolTable()->defineSymbol(sectionName, index, 0, SymbolTable::SYMB_LOC);
}

Section::~Section() {
    for (int i = 0; i < lines.size(); i++)
        delete lines[i];
    for (int i = 0; i < relocations.size(); i++)
        delete relocations[i];
}

void Section::addLine(Line* line, bool fromPool) {
    lines.push_back(line);
    if (fromPool)
        dispFixupIndexes.push_back(lines.size() - 1);
}

int Section::addLiteralPoolValue(int value, const char* symbol) {
    std::string key = (symbol != nullptr ? symbol : std::to_string(value));

    // if the value is already added to the pool
    // return it's slot
    auto it = literalPoolIndex.find(key);
    if (it != literalPoolIndex.end())
        return it->second;

    // add new entry
    LitPoolEntry* e = new LitPoolEntry();
    e->value = value;
    e->symbol = (symbol != nullptr ? symbol : "");
    literalPool.push_back(e);

    // todo: add relocation entry if the 
    // symbol isn't defined

    int idx = literalPool.size() - 1;
    literalPoolIndex[key] = idx;
    return idx;
}

int Section::addRelocation(int offset, RelocType type , std::string symbol, int addend) {
    RelocEntry* e = new RelocEntry();
    e->offset = offset;
    e->type = type;
    e->symbol = symbol;
    e->addend = addend;
    relocations.push_back(e);
    return relocations.size() - 1;
}

void Section::backpatch() {
    // for each entry in forward reference table:
    //      if entry.symbol is defined:
    //          patch location with correct value
    //      else:
    //          generate a relocation entry, mark the symbol UND in symbol table
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (auto& e : freftab) {
        std::string symbol = e.first;
        if (symtab->isDefined(symbol)) {
            int value = symtab->getSymbolValue(symbol);
            for (int i = 0; i < e.second.size(); i++) {
                // todo: patch...
            }
        }
        else {
            // generate relocation entry
            symtab->declareSymbolExtern(symbol);
        }
    }
}

int Section::getSectionID() { return index; }

void Section::serialize(std::ofstream& out) {
    // serialize section
    std::vector<uint8_t> section_bytes;
    for (Line* l : lines) {
        std::vector<uint8_t> bytes = l->generateBytes();
        for (uint8_t byte : bytes) 
            section_bytes.push_back(byte);
    }

    // patch displacements to literal pool values 

    int litpool_start = section_bytes.size();
    if (litpool_start >= (1 << 11) 
        || litpool_start < -(1 << 11)) {

        // todo: throw error
        std::cout << "error: section " << name << "is too large!\n";
        return;
    }

    for (int ind : dispFixupIndexes) {
        // displacement field is the lowest 12b 
        // of an instruction

        // section_bytes[4*k] | section_bytes[4*k + 1] | section_bytes[4*k + 2] | section_bytes[4*k + 3]
        int disp = ((section_bytes[4 * ind + 2] & 0xf) << 8) + section_bytes[4 * ind + 3] + litpool_start;
        section_bytes[4 * ind + 3] = disp & 0xff;

        section_bytes[4 * ind + 2] &= 0xf0;         // clear lower 4 bits
        section_bytes[4 * ind + 2] |= disp >> 8;    // append upper 4 bits of disp
    }

    // serialize literal pool
    // values are stored in little endian format
    // if a symbol referenced in the pool isn't
    // defined at this point, a relocation entry
    // is generated

    int mask = 0xffff;
    int value;
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (LitPoolEntry* l : literalPool) {
        if (!symtab->isDefined(l->symbol)) {
            // todo: add relocation entry
            value = 0;
        }
        else value = l->value;
        for (int i = 0; i < 4; i++) {
            section_bytes.push_back(value & mask);
            value >>= 8;
        }
    }

    // write buffer to output

    for (uint8_t byte : section_bytes) 
        out << byte;
}