#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"
#include "../inc/interface.h"

#include <iostream>
#include <iomanip>

// sections are numbered starting from 1
int Section::counter = 1;

Section::Section(std::string sectionName, int offset) 
    : index(counter++) 
    , name(sectionName)
    , offset(offset)
    , startAddress(0) {
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

    // if symbol isn't absolute, add a 
    // foward reference table entry
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    if (symbol != nullptr && !symtab->isAbsolute(e->symbol)) 
        addForwardReference(e->symbol, location_counter);

    int idx = literalPool.size() - 1;
    literalPoolIndex[key] = idx;
    return idx;
}

int Section::addRelocation(int offset, RelocType type , int symbolIndex, int addend) {
    RelocEntry* e = new RelocEntry();
    e->offset = offset;
    e->type = type;
    e->symbol = symbolIndex;
    e->addend = addend;
    relocations.push_back(e);
    return relocations.size() - 1;
}

void Section::addForwardReference(std::string symbol, int location) {
    freftab[symbol].push_back(location);
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
        int value = 0;

        for (int i = 0; i < e.second.size(); i++) {
            value = symtab->getSymbolValue(symbol);
            if (symtab->isAbsolute(symbol)) {
                // b4 | b3 | b2 | b1
                int b1 = value & 0xff;
                int b2 = (value >> 8) & 0xff;
                int b3 = (value >> 16) & 0xff;
                int b4 = (value >> 24) & 0xff;

                section_bytes[e.second[i]] = b4;
                section_bytes[e.second[i] + 1] = b3;
                section_bytes[e.second[i] + 2] = b2;
                section_bytes[e.second[i] + 3] = b1;
            }
            else {
                if (!symtab->isDefined(symbol))
                    symtab->declareSymbolExtern(symbol);

                if (symtab->getSymbolBind(symbol) == SymbolTable::SYMB_GLOB) {
                    addRelocation(
                        e.second[i],
                        ABS,
                        symtab->getSymbolIndex(symbol),
                        0
                    );
                }
                else {
                    int symb_sec_id = symtab->getSymbolSection(symbol);
                    std::string symb_sec_name = ObjectFile::getSectionFromID(symb_sec_id);
                    addRelocation(
                        e.second[i],
                        REL,
                        symtab->getSymbolIndex(symb_sec_name),
                        // symtab->getSymbolIndex(symbol),
                        value 
                    );
                }
            }
        }
    }
}

int Section::getSectionID() { return index; }

std::string Section::getSectionName() { return name; }

void Section::serialize(std::ofstream& out) {
    // serialize section

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

        // instruction format:
        //  > section_bytes[4*k] | section_bytes[4*k + 1] | section_bytes[4*k + 2] | section_bytes[4*k + 3]

        int disp = ((section_bytes[4 * ind + 2] & 0xf) << 8) + section_bytes[4 * ind + 3]       // offset in literal pool
                    + (litpool_start - 4 * ind)                 // distance from the instruction to literal pool
                    - 4;                                        // when executing PC points to next instruction


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
    std::vector<uint8_t> litpool_bytes;
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (int i = 0; i < literalPool.size(); i++) {
        LitPoolEntry* l = literalPool[i];

        if (!l->symbol.empty() && !symtab->isDefined(l->symbol)) {
            addRelocation(
                litpool_start + 4*i,
                RelocType::ABS,
                symtab->getSymbolIndex(l->symbol),
                0
            );
            value = 0;
        }
        else value = l->value;

        // serialize
        for (int j = 0; j < 4; j++) {
            litpool_bytes.push_back(value & mask);
            value >>= 8;
        }
    }

    backpatch();

    // write section contents to output
    bool newline = false;
    out << "#" << name << "\n";
    int size = section_bytes.size();
    for (int i = 0; i < size; i += 4) {
        out << std::hex << std::setw(2) << (int)section_bytes[i] << " ";
        
        if (i + 1 < size)
            out << std::hex << std::setw(2) << (int)section_bytes[i + 1] << " ";
        if (i + 2 < size)
            out << std::hex << std::setw(2) << (int)section_bytes[i + 2] << " ";
        if (i + 3 < size)
            out << std::hex << std::setw(2) << (int)section_bytes[i + 3] << " ";

        if (newline) out << "\n";
        else out << "    ";

        newline = !newline;
    }

    if (newline) {
        out << "\n";
        newline = false;
    }

    // write literal pool contents to output
    out << "#" << name << ".litpool\n";
    for (int i = 0; i < litpool_bytes.size(); i += 4) {
        out << std::hex << std::setw(2) << (int)litpool_bytes[i];
        out << " ";
        out << std::hex << std::setw(2) << (int)litpool_bytes[i + 1];
        out << " ";
        out << std::hex << std::setw(2) << (int)litpool_bytes[i + 2];
        out << " ";
        out << std::hex << std::setw(2) << (int)litpool_bytes[i + 3];
        if (newline) out << "\n";
        else out << "    ";
        newline = !newline;
    }

    if (newline) {
        out << "\n";
        newline = false;
    }

    // write relocation entries to output
    const int offsetWidth = 6;
    const int typeWidth = 5;
    const int symbolWidth = 10;
    const int addendWidth = 10;

    out << std::dec << "#" << name << ".rela\n";
    out << std::left
        << std::setw(offsetWidth) << "Offset" << " | "
        << std::setw(typeWidth) << "Type" << " | "
        << std::setw(symbolWidth) << "Symbol" << " | "
        << std::setw(addendWidth) << "Addend" << "\n";

    for (RelocEntry* rela : relocations) {
        std::string type = "ABS";
        if (rela->type == REL) type = "REL";

        out << std::left
            << std::setw(offsetWidth) << rela->offset << " | "
            << std::setw(typeWidth) << type << " | "
            << std::setw(symbolWidth) << rela->symbol << " | "
            << std::setw(addendWidth) << rela->addend << "\n";
    }
}