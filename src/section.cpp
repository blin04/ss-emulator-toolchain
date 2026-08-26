#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

#include <iostream>
#include <iomanip>

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

void Section::backpatch() {
    // for each entry in forward reference table:
    //      if entry.symbol is defined:
    //          patch location with correct value
    //      else:
    //          generate a relocation entry, mark the symbol UND in symbol table
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (auto& e : freftab) {
        std::string symbol = e.first;
        bool defined = symtab->isDefined(symbol);
        int value = 0;

        if (defined) value = symtab->getSymbolValue(symbol);
        else symtab->declareSymbolExtern(symbol);

        for (int i = 0; i < e.second.size(); i++) {
            if (value) {
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
                addRelocation(
                    e.second[i],
                    ABS,
                    symtab->getSymbolIndex(symbol),
                    0
                );
            }
        }
    }
}

int Section::getSectionID() { return index; }

void Section::serialize(std::ofstream& out) {
    // serialize section

    out << "#." << name << "\n";
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
    std::vector<uint8_t> litpool_bytes;
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (int i = 0; i < literalPool.size(); i++) {
        LitPoolEntry* l = literalPool[i];
        
        if (!symtab->isDefined(l->symbol)) {
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
    out << "#." << name << "\n";
    for (int i = 0; i < section_bytes.size(); i += 4) {
        out << std::hex << std::setw(2) << (int)section_bytes[i];
        out << " ";
        out << std::hex << std::setw(2) << (int)section_bytes[i + 1];
        out << " ";
        out << std::hex << std::setw(2) << (int)section_bytes[i + 2];
        out << " ";
        out << std::hex << std::setw(2) << (int)section_bytes[i + 3];
        if (newline) out << "\n";
        else out << "    ";
        newline = !newline;
    }

    if (newline) {
        out << "\n";
        newline = false;
    }

    // write literal pool contents to output
    out << "#." << name << ".litpool\n";
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
    out << "#." << name << ".rela\n";

    const int offsetWidth = 6;
    const int typeWidth = 5;
    const int symbolWidth = 20;
    const int addendWidth = 10;

    out << "#." << name << ".rela\n";
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