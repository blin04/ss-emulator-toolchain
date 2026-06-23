#include <iostream>

#include "../inc/directives.hpp"
#include "../inc/interface.h"
#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

void defineSymbol(const char* name, int value, bool equ_defined) {
    // section id is 0 for now
    std::cout << "defined " << name << " with value " << value << "\n";
    ObjectFile::getSymbolTable()->defineSymbol(name, 0, value, 
        equ_defined ? SymbolTable::SYMB_ABS : SymbolTable::SYMB_LOC);
}

void declareSymbolsGlobal(char** symbs) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (int i = 0; symbs[i] != nullptr; i++) {
        symtab->declareSymbolGlobal(symbs[i]);
        free(symbs[i]);
    }
}

void declareSymbolsExtern(char** symbs) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    for (int i = 0; symbs[i] != nullptr; i++) {
        symtab->declareSymbolExtern(symbs[i]);
        free(symbs[i]);
    }
}

int getSymbolValue(const char* name) {
    return ObjectFile::getSymbolTable()->getSymbolValue(name);
}

bool isDefined(const char* symbol) { return ObjectFile::getSymbolTable()->isDefined(symbol); }

bool isExtern(const char* symbol) { return ObjectFile::getSymbolTable()->isExtern(symbol); }

void startNewSection(const char* name) {
    std::cout << "creating section named " << name << "\n";
    ObjectFile::getInstance()->newSection(name);
}

void addInstruction() {
    std::cout << "called addInstruction()\n";
}

void addDirective() {
    std::cout << "called addDirective()\n";
}

void addAsciiDirective(const char* str) {
    Section* curr = ObjectFile::getCurrentSection();
    curr->addLine(new AsciiDirective(str));
}

void addSkipDirective(int bytes_count) {
    std::cout << "adding skip directive that skips " << bytes_count << " bytes\n";
    Section* curr = ObjectFile::getCurrentSection();
    curr->addLine(new SkipDirective(bytes_count));
}

// returns size of allocated memory space 
// for initializers in order to more
// efficiently update location counter
int addWordDirective(char** initializers) {
    Section* curr = ObjectFile::getCurrentSection();
    std::vector<std::string> initializers_param;

    int i;
    for (i = 0; initializers[i] != nullptr; i++) {
        initializers_param.push_back(initializers[i]);
        free(initializers[i]);
    }
    WordDirective* w = new WordDirective(initializers_param);
    curr->addLine(new WordDirective(initializers_param));
    free(initializers);
    return 4 * i;
}
