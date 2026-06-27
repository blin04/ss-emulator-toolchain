#include <fstream>

#include <iostream>

#include "../inc/freftab.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

ObjectFile::ObjectFile() {
    currentSection = nullptr;
    symbolTable = new SymbolTable();
    freftab = new ForwardReferenceTable();
}

ObjectFile::~ObjectFile() {
    for (int i = 0; i < sections.size(); i++)
        delete sections[i];
    delete symbolTable;
}

void ObjectFile::backpatch() {
    // for each entry in forward reference table:
    //      if entry.symbol is defined:
    //          patch location with correct value
    //      else:
    //          generate a relocation entry, mark the symbol UND in symbol table
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
 
ForwardReferenceTable* ObjectFile::getForwardReferenceTable() {
    return getInstance()->freftab;
}

void ObjectFile::newSection(std::string name, int offset) {
    if (currentSection != nullptr)
        sections.push_back(currentSection);
    Section* s = new Section(name, offset); 
    currentSection = s;
}

void ObjectFile::generate() {
    if (currentSection != nullptr) 
        sections.push_back(currentSection);

    // file name hardcoded for now
    std::ofstream out("main.obj", std::ios::out | std::ios::binary);

    // symbolTable->serialize(out);

    // todo: from sections vector extract
    // data needed for Section Table and
    // serialize it

    for (Section*& s : sections) {
        s->serialize(out);    
    }

    out.close();
}

void ObjectFile::printSymbolTable() { std::cout << "Symbol table for file ...: \n"; symbolTable->print(); }