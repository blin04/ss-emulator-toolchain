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
}

void Section::addLine(Line* line) {
    lines.push_back(line);
}

int Section::getSectionID() { return index; }

void Section::serialize(std::ofstream& out) {
    for (Line* l : lines) {
        std::vector<uint8_t> bytes = l->generateBytes();
        for (uint8_t byte : bytes) out << byte;
    }
}