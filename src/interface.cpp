#include <iostream>

#include "../inc/directives.hpp"
#include "../inc/freftab.hpp"
#include "../inc/interface.h"
#include "../inc/instruction.hpp"
#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"
#include "../misc/parser.tab.h"

int location_counter = 0;

// defines symbol with a particular value
void defineSymbol(const char* name, bool equ_defined) {
    ObjectFile::getSymbolTable()->defineSymbol(
        name, 
        ObjectFile::getCurrentSection()->getSectionID(), 
        location_counter, 
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
    }
}

void oneOpJumpStatementHandler(int stmt, Operand op) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    bool fromPool = false;

    // if the value of the symbol can in any way
    // (it's not absolute or it's not known) exceed 
    // 12b, the symbol gets added to the literal pool

    // forward reference table entry is generated
    // for symbols that are not defined

    // relocation entry is generated
    // for symbols that aren't absolute



    if (op.symbol != nullptr && symtab->isDefined(op.symbol)) {
        // add entry to forward reference table
        ForwardReferenceTable* freftab = ObjectFile::getForwardReferenceTable();
        freftab->addEntry(op.symbol, location_counter);
        free(op.symbol);        // free memory allocated by strdup
        fromPool = true;        // the symbol value might be larger than 12b
    }
    else {
        // check if the disp can fit into 12b
        // if not, add entry to literal pool
        if (op.disp >= 2048 || op.disp < -2048) 
            fromPool = true;
        // todo: add entry to literal pool
    }

    if (fromPool) {
        Section* currSection = ObjectFile::getCurrentSection();
        op.disp = currSection->addLiteralPoolValue(op.disp) * 4;
        // todo: add forward reference table entry that 
        // should correct the displacement value since
        // location of the literal pool isn't known
    }

    // todo: if symbol is stored into literal pool
    // offset to it from current location must be 
    // passed instead of `op.disp`

    switch (stmt) {
        case yytoken_kind_t::JMP:
            Instruction::jmpHandler(op.disp, fromPool);
            break;
        case yytoken_kind_t::CALL:
            Instruction::callHandler(op.disp, fromPool);
            break;
    }

    if (op.symbol != nullptr)
        free(op.symbol);
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

void threeOpStatementHandler(int stmt, int gpr1, int gpr2, Operand op) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    bool fromPool = false;
    if (op.symbol != nullptr && !symtab->isDefined(op.symbol)) {
        // add entry to forward reference table
        ForwardReferenceTable* freftab = ObjectFile::getForwardReferenceTable();
        freftab->addEntry(op.symbol, location_counter);
        free(op.symbol);        // free memory allocated by strdup
        fromPool = true;        // the symbol value might be larger than 12b
    }
    else {
        // check if the disp can fit into 12b
        // if not add entry to literal pool
        if (op.disp >= 2048 || op.disp < -2048) 
            fromPool = true;
        // todo: add entry to literal pool
    }

    // todo: if symbol is stored into literal pool
    // offset to it from current location must be 
    // passed instead of `op.disp`

    switch (stmt) {
        case yytoken_kind_t::BEQ:
            Instruction::beqHandler(gpr1, gpr2, op.disp, fromPool);
            break;
        case yytoken_kind_t::BNE:
            Instruction::beqHandler(gpr1, gpr2, op.disp, fromPool);
            break;
        case yytoken_kind_t::BGT:
            Instruction::beqHandler(gpr1, gpr2, op.disp, fromPool);
            break;
    }
}

void memoryStatementHandler(int type, Operand op, int gpr) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    bool fromPool = false;
    if (op.symbol != nullptr && !symtab->isDefined(op.symbol)) {
        // add entry to forward reference table
        ForwardReferenceTable* freftab = ObjectFile::getForwardReferenceTable();
        freftab->addEntry(op.symbol, location_counter);
        free(op.symbol);        // free memory allocated by strdup
        fromPool = true;
    }
    else {
        // check if the disp can fit into 12b
        // if not, add entry to literal pool
        if (op.disp >= 2048 || op.disp < -2048) 
            fromPool = true;
        // todo: add entry for literal pool
    }

    // todo: if symbol is stored into literal pool
    // offset to it from current location must be 
    // passed instead of `op.disp`

    switch (type) {
        case yytoken_kind_t::LD:
            Instruction::ldHandler(op.fromMemory, op.gpr, op.disp, gpr, fromPool);
            break;
        case yytoken_kind_t::ST:
            Instruction::stHandler(op.fromMemory, op.gpr, op.disp, gpr, fromPool);
            break;
    }
}