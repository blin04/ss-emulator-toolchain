#include <iostream>

#include "../inc/directives.hpp"
#include "../inc/interface.h"
#include "../inc/instruction.hpp"
#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"
#include "../misc/parser.tab.h"

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

void zeroOpStatementHandler(int stmt) {
    switch (stmt) {
        case yytoken_kind_t::HALT:
            Instruction::haltHandler();
            break;
        case yytoken_kind_t::INT:
            Instruction::intHandler();
            break;
        case yytoken_kind_t::IRET:
            Instruction::iretHandler();
            break;
        case yytoken_kind_t::RET:
            Instruction::retHandler();
            break;
    }
}

void oneOpStatementHandler(int stmt, int op) {
    switch (stmt) {
        case yytoken_kind_t::NOT:
            Instruction::notHandler(op);
            break;
        case yytoken_kind_t::PUSH:
            Instruction::pushHandler(op);
            break;
        case yytoken_kind_t::POP:
            Instruction::popHandler(op);
            break;
        case yytoken_kind_t::JMP:
            Instruction::jmpHandler(op);
            break;
        case yytoken_kind_t::CALL:
            Instruction::callHandler(op);
            break;
    }
}

void twoOpStatementHandler(int stmt, int op1, int op2) {
    switch (stmt) {
        case yytoken_kind_t::ADD:
            Instruction::addHandler(op1, op2);
            break;
        case yytoken_kind_t::SUB:
            Instruction::subHandler(op1, op2);
            break;
        case yytoken_kind_t::MUL:
            Instruction::mulHandler(op1, op2);
            break;
        case yytoken_kind_t::DIV:
            Instruction::divHandler(op1, op2);
            break;
        case yytoken_kind_t::AND:
            Instruction::andHandler(op1, op2);
            break;
        case yytoken_kind_t::OR:
            Instruction::orHandler(op1, op2);
            break;
        case yytoken_kind_t::XOR:
            Instruction::xorHandler(op1, op2);
            break;
        case yytoken_kind_t::SHL:
            Instruction::shlHandler(op1, op2);
            break;
        case yytoken_kind_t::SHR:
            Instruction::shrHandler(op1, op2);
            break;
        case yytoken_kind_t::XCHNG:
            Instruction::xchngHandler(op1, op2);
            break;
        case yytoken_kind_t::CSRRD:
            Instruction::csrrdHandler(op1, op2);
            break;
        case yytoken_kind_t::CSRWR:
            Instruction::csrwrHandler(op1, op2);
            break;
    }
}

void threeOpStatementHandler(int stmt, int gpr1, int gpr2, int op) {
    switch (stmt) {
        case yytoken_kind_t::BEQ:
            Instruction::beqHandler(gpr1, gpr2, op);
            break;
        case yytoken_kind_t::BNE:
            Instruction::beqHandler(gpr1, gpr2, op);
            break;
        case yytoken_kind_t::BGT:
            Instruction::beqHandler(gpr1, gpr2, op);
            break;
    }
}