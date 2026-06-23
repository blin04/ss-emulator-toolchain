#include <iostream>

#include "../inc/directives.hpp"
#include "../inc/interface.h"
#include "../inc/instruction.hpp"
#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

void defineSymbol(const char* name, int value, bool equ_defined) {
    // section id is 0 for now
    std::cout << "defined " << name << " with value " << value << "\n";
    ObjectFile::getSymbolTable()->defineSymbol(
        name, 
        ObjectFile::getCurrentSection()->getSectionID(), 
        value, 
        equ_defined ? SymbolTable::SYMB_ABS : SymbolTable::SYMB_LOC
    );
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

void startNewSection(const char* name, int offset) {
    std::cout << "creating section named " << name << "\n";
    ObjectFile::getInstance()->newSection(name, offset);
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

void haltHandler() {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0, 0, 0, 0, 0, 0)
    );
}

void intHandler() {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(1, 0, 0, 0, 0, 0)
    );
}

void iretHandler() {
    // pop pc
    uint8_t inst = 0b1001;        // data loading instr
    uint8_t mode = 0b0011;
    uint8_t a = Instruction::GPR::PC;
    uint8_t b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 0)
    );

    // pop status
    mode = 0b0111;
    a = Instruction::CSR::status;
    b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 0)
    );
}

void retHandler() {
    // pop pc
    uint8_t inst = 0b1001;        // data loading instr
    uint8_t mode = 0b0011;
    uint8_t a = Instruction::GPR::PC;
    uint8_t b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 0)
    );
}