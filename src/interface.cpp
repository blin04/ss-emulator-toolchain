#include <iostream>

#include "../inc/directives.hpp"
#include "../inc/interface.h"
#include "../inc/instruction.hpp"
#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"
#include "../misc/parser.tab.h"

int location_counter = 0;

bool handleOperand(Operand &op) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    bool fromPool = false;

    // if the value of the symbol can in any way
    // (it's not absolute or it's not defined) exceed 
    // 12b, the symbol gets added to the literal pool

    // forward reference table entry is generated
    // for symbols that are not defined

    // relocation entry is generated
    // for symbols that aren't absolute

    if (op.symbol != nullptr && !symtab->isDefined(op.symbol)) {
        // symbol isn't defined, so it's entirely possible
        // that is runtime value exceeds 12b
        fromPool = true;        
    }
    else {
        // branch is entered if either a defined symbol 
        // or a literal are encountered
        // the corresponding value is stored in `disp` field
        // if disp can't fit into 12b, it must be stored in
        // literal pool 
        if (op.disp >= 2048 || op.disp < -2048) 
            fromPool = true;
    }

    if (fromPool) {
        // `disp` field equals to offset in bytes from 
        // the start of literal pool to the added value
        // this makes it easy to patch it up once the 
        // literal pool start address is known - the
        // star address only needs to be added to 
        // the stored displacement value
        // relocation entry, if needed, is generated 
        // during literal pool serialization
        op.disp = ObjectFile::getCurrentSection()->addLiteralPoolValue(op.disp, op.symbol) * 4;
    }
    else {
        // a relocation entry for a symbol 
        // that's referenced absolutely
        // must be generated
        if (op.symbol != nullptr && !symtab->isAbsolute(op.symbol)) {
            // todo: add relocation entry
            ObjectFile::getCurrentSection()->addRelocation(
                location_counter,
                RelocType::ABS,
                symtab->getSymbolIndex(op.symbol),
                0 
            ); 
        }
    }
    return fromPool;
}

// defines symbol with a particular value
void defineSymbol(const char* name, int value, bool equ_defined) {
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
    location_counter = 0;       // resets the location counter
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

/*
*   Handles: [call | jmp] <operand>
*/
void oneOpJumpStatementHandler(int stmt, Operand op) {

    bool fromPool = handleOperand(op);

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

/*
*   Handles: [beq | bne | bgt] <gpr1>, <gpr2>, <operand>
*/
void threeOpStatementHandler(int stmt, int gpr1, int gpr2, Operand op) {

    bool fromPool = handleOperand(op);

    switch (stmt) {
        case yytoken_kind_t::BEQ:
            Instruction::beqHandler(gpr1, gpr2, op.disp, fromPool);
            break;
        case yytoken_kind_t::BNE:
            Instruction::bneHandler(gpr1, gpr2, op.disp, fromPool);
            break;
        case yytoken_kind_t::BGT:
            Instruction::bgtHandler(gpr1, gpr2, op.disp, fromPool);
            break;
    }

    if (op.symbol != nullptr)
        free(op.symbol);
}

/*
*   Handles: ld <operand>, <gpr> | st <gpr>, <operand>
*/
void memoryStatementHandler(int type, Operand op, int gpr) {

    bool fromPool = handleOperand(op);

    switch (type) {
        case yytoken_kind_t::LD:
            Instruction::ldHandler(op.fromMemory, op.gpr, op.disp, gpr, fromPool);
            break;
        case yytoken_kind_t::ST:
            Instruction::stHandler(op.fromMemory, op.gpr, op.disp, gpr, fromPool);
            break;
    }

    if (op.symbol != nullptr)
        free(op.symbol);
}