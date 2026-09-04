#include <fstream>

#include <iostream>

#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

ObjectFile::ObjectFile() 
    : currentSection(nullptr)
    , symbolTable(new SymbolTable()) 
    , outputPath("asm_out.txt") 
    {}

ObjectFile::~ObjectFile() {
    for (int i = 0; i < sections.size(); i++)
        delete sections[i];
    delete symbolTable;
}

ObjectFile* ObjectFile::getInstance() {
    static ObjectFile instance;
    return &instance;
}

Section* ObjectFile::getCurrentSection() {
    return getInstance()->currentSection;
}

SymbolTable* ObjectFile::getSymbolTable() {
    return getInstance()->symbolTable;
}

std::string ObjectFile::getSectionFromID(int id) {
    return getInstance()->sections[id - 1]->getSectionName();
}
 
void ObjectFile::newSection(std::string name, int offset) {
    if (currentSection != nullptr)
        sections.push_back(currentSection);
    Section* s = new Section("." + name, offset); 
    symbolTable->defineSymbol(
        "." + name, 
        s->getSectionID(), 
        0, 
        SymbolTable::SYMB_LOC 
    );
    currentSection = s;
}

void ObjectFile::generate() {
    if (currentSection != nullptr) 
        sections.push_back(currentSection);

    // file name hardcoded for now
    std::ofstream out(outputPath, std::ios::out);

    symbolTable->serialize(out);

    // todo: from sections vector extract
    // data needed for Section Table and
    // serialize it

    for (Section*& s : sections) {
        s->serialize(out);    
    }

    out.close();
}

void ObjectFile::setOutput(std::string path) {
    outputPath = path;
}