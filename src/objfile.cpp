#include <fstream>

#include <iostream>

#include "../inc/interface.h"
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
    Section* s = new Section(name, offset); 
    symbolTable->defineSymbol(
        name, 
        s->getSectionID(), 
        0, 
        SymbolTable::SYMB_LOC 
    );
    currentSection = s;
}

void ObjectFile::generate() {
    try {
        if (currentSection != nullptr) 
            sections.push_back(currentSection);

        // resolve deferred .equ definitions now that the whole file is parsed
        if (!resolvePendingEqus())
            return;

        for (Section*& s : sections)
            s->generateContent();

        std::ofstream out(outputPath, std::ios::out);
        
        symbolTable->serialize(out);
        for (Section*& s : sections)
            s->serialize(out);    

        out.close();
    }
    catch (std::exception& e) {
        std::cout << e.what() << "\n";
        return;
    }
}

void ObjectFile::setOutput(std::string path) {
    outputPath = path;
}

void ObjectFile::addPendingEqu(const char* name, Expr* expr) {
    pendingEqus.push_back({ name, expr });
}

bool ObjectFile::resolvePendingEqus() {
    bool progress = true;
    while (progress && !pendingEqus.empty()) {
        progress = false;
        for (auto it = pendingEqus.begin(); it != pendingEqus.end(); ) {
            int value;
            EquKind equ_kind = classifyEqu(it->second, value);

            if (equ_kind == EQU_DEFER) {
                // dependency can't be resolved yet, defer
                ++it;
                continue;
            }

            if (equ_kind == EQU_ERROR) {
                std::cout << "error: invalid .equ expression for symbol " << it->first << "\n";
                return false;
            }

            defineSymbol(it->first.c_str(), value, true);
            delete it->second;
            it = pendingEqus.erase(it);
            progress = true;
        }
    }

    if (!pendingEqus.empty()) {
        for (auto& e : pendingEqus)
            std::cout << "error: undefined symbols referenced in definition of " << e.first << "\n";
        return false;
    }
    return true;
}