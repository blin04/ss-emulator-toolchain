#include <fstream>

#include <iostream>

#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

ObjectFile::ObjectFile() {
    currentSection = nullptr;
    symbolTable = new SymbolTable();
}

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

void ObjectFile::newSection(std::string name) {
    if (currentSection != nullptr)
        sections.push_back(currentSection);
    Section* s = new Section(name); 
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