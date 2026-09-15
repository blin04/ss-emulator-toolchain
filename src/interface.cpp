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
    // an operand might be a `data_operand` or a `jump_operand`
    // in any case, it might reference a literal, a symbol 
    // or none of them

    // value of literals and symbols might be directly 
    // encoded into the instruction's displacement field
    // or it might be placed into the literal pool, with
    // the instruction accessing it using PC relative 
    // addressing
    //
    // direct encoding is performed *only* in the 
    // following situations:
    //  > literal - the value fits into 12b
    //  > symbol - it's absolute (.equ defined)
    //             and it's vlaue fits into 12b
    // in that case, no action is required from
    // this function
    //
    // in all the other cases, the value is placed
    // into the literal pool

    bool fits = (op.disp < (1 << 12));
    SymbolTable* symtab = ObjectFile::getSymbolTable();

    if ((op.symbol == nullptr && fits) 
        || (op.symbol != nullptr && symtab->isAbsolute(op.symbol) && fits))
        return false;


    // `disp` field equals to offset in bytes from 
    // the start of literal pool to the added value
    // this makes it easy to patch it up once the 
    // literal pool start address is known - the
    // start address only needs to be added to 
    // the stored displacement value
    // relocation entry, if needed, is generated 
    // during literal pool serialization
    op.disp = ObjectFile::getCurrentSection()->addLiteralPoolValue(op.disp, op.symbol) * 4;
    return true;
}

// defines symbol with a particular value
void defineSymbol(const char* name, int value, bool abs, int sectionID) {
    int section;
    if (abs)
        section = SymbolTable::SYMB_ABS;
    else if (sectionID != -1)
        section = sectionID;
    else
        section = ObjectFile::getCurrentSection()->getSectionID();

    ObjectFile::getSymbolTable()->defineSymbol(
        name,
        section,
        value,
        SymbolTable::SYMB_LOC,
        abs
    );
}

void defineEquSymbol(const char* name, Expr* expr) {
    int value;
    EquKind equ_kind = classifyEqu(expr, value);
    if (equ_kind != EQU_DEFER && equ_kind != EQU_ERROR) {
        defineSymbol(name, value, (equ_kind == EQU_ABSOLUTE));
    }
    else {
        // some symbols undefined, defer resolvement
        // error cases are also deffered because handling
        // is performed later
        ObjectFile::getInstance()->addPendingEqu(name, expr);
    }
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

bool isAbsolute(const char* symbol) { return ObjectFile::getSymbolTable()->isAbsolute(symbol); }

bool isDefined(const char* symbol) { return ObjectFile::getSymbolTable()->isDefined(symbol); }

bool isExtern(const char* symbol) { return ObjectFile::getSymbolTable()->isExtern(symbol); }

void startNewSection(const char* name, int offset) {
    ObjectFile::getInstance()->newSection(name, offset);
    location_counter = 0;       // resets the location counter
}

void addAsciiDirective(const char* str) {
    Section* curr = ObjectFile::getCurrentSection();
    curr->addLine(new AsciiDirective(str));
}

void addSkipDirective(int bytes_count) {
    Section* curr = ObjectFile::getCurrentSection();
    curr->addLine(new SkipDirective(bytes_count));
}

// returns size of allocated memory space 
// for initializers in order to more
// efficiently update location counter
int addWordDirective(char** initializers) {
    Section* sec = ObjectFile::getCurrentSection();
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    std::vector<long> params;

    int i;
    long value;
    for (i = 0; initializers[i] != nullptr; i++) {
        if ((initializers[i][0] - '0') < 10 && (initializers[i][0] - '0') >= 0) 
            value = std::stol(initializers[i]);
        else {
            // note: used for adding symbol into the table as undefined
            value = (long)ObjectFile::getSymbolTable()->getSymbolValue(initializers[i]);
            if (!symtab->isDefined(initializers[i]))
                sec->addForwardReference(initializers[i], location_counter + 4* i);
        }
        params.push_back(value);
        free(initializers[i]);
    }
    sec->addLine(new WordDirective(params));
    free(initializers);
    return 4 * i;
}

int zeroOpStatementHandler(int stmt) {
    switch (stmt) {
        case yytoken_kind_t::HALT:
            return Instruction::haltHandler();
        case yytoken_kind_t::INT:
            return Instruction::intHandler();
        case yytoken_kind_t::IRET:
            return Instruction::iretHandler();
        case yytoken_kind_t::RET:
            return Instruction::retHandler();
    }
    return 0;
}

int oneOpStatementHandler(int stmt, int op) {
    switch (stmt) {
        case yytoken_kind_t::NOT:
            return Instruction::notHandler(op);
        case yytoken_kind_t::PUSH:
            return Instruction::pushHandler(op);
        case yytoken_kind_t::POP:
            return Instruction::popHandler(op);
    }
    return 0;
}

/*
*   Handles: [call | jmp] <operand>
*/
int oneOpJumpStatementHandler(int stmt, Operand op) {

    bool fromPool = handleOperand(op);
    int count = 0;

    switch (stmt) {
        case yytoken_kind_t::JMP:
            count = Instruction::jmpHandler(op.disp, fromPool);
            break;
        case yytoken_kind_t::CALL:
            count = Instruction::callHandler(op.disp, fromPool);
            break;
    }

    if (op.symbol != nullptr)
        free(op.symbol);

    return count;
}

int twoOpStatementHandler(int stmt, int op1, int op2) {
    switch (stmt) {
        case yytoken_kind_t::ADD:
            return Instruction::addHandler(op1, op2);
        case yytoken_kind_t::SUB:
            return Instruction::subHandler(op1, op2);
        case yytoken_kind_t::MUL:
            return Instruction::mulHandler(op1, op2);
        case yytoken_kind_t::DIV:
            return Instruction::divHandler(op1, op2);
        case yytoken_kind_t::AND:
            return Instruction::andHandler(op1, op2);
        case yytoken_kind_t::OR:
            return Instruction::orHandler(op1, op2);
        case yytoken_kind_t::XOR:
            return Instruction::xorHandler(op1, op2);
        case yytoken_kind_t::SHL:
            return Instruction::shlHandler(op1, op2);
        case yytoken_kind_t::SHR:
            return Instruction::shrHandler(op1, op2);
        case yytoken_kind_t::XCHNG:
            return Instruction::xchngHandler(op1, op2);
        case yytoken_kind_t::CSRRD:
            return Instruction::csrrdHandler(op1, op2);
        case yytoken_kind_t::CSRWR:
            return Instruction::csrwrHandler(op1, op2);
    }
    return 0;
}

/*
*   Handles: [beq | bne | bgt] <gpr1>, <gpr2>, <operand>
*/
int threeOpStatementHandler(int stmt, int gpr1, int gpr2, Operand op) {

    bool fromPool = handleOperand(op);
    int count = 0;

    switch (stmt) {
        case yytoken_kind_t::BEQ:
            count = Instruction::beqHandler(gpr1, gpr2, op.disp, fromPool);
            break;
        case yytoken_kind_t::BNE:
            count = Instruction::bneHandler(gpr1, gpr2, op.disp, fromPool);
            break;
        case yytoken_kind_t::BGT:
            count = Instruction::bgtHandler(gpr1, gpr2, op.disp, fromPool);
            break;
    }

    if (op.symbol != nullptr)
        free(op.symbol);

    return count;
}

/*
*   Handles: ld <operand>, <gpr> | st <gpr>, <operand>
*/
int memoryStatementHandler(int type, Operand op, int gpr) {

    bool fromPool = handleOperand(op);
    int count = 0;

    switch (type) {
        case yytoken_kind_t::LD:
            count = Instruction::ldHandler(op.fromMemory, op.gpr, op.disp, gpr, fromPool);
            break;
        case yytoken_kind_t::ST:
            count = Instruction::stHandler(op.fromMemory, op.gpr, op.disp, gpr, fromPool);
            break;
    }

    if (op.symbol != nullptr)
        free(op.symbol);

    return count;
}

Expr* exprLiteral(int v) {
    Expr* e = new Expr();
    e->constValue = v;
    return e;
}

Expr* exprSymbol(const char* s) {
    Expr* e = new Expr();
    e->terms.push_back({ 1, s });
    return e;
}

Expr* exprAdd(Expr* a, Expr* b) {
    a->constValue += b->constValue;
    for (auto& t : b->terms)
        a->terms.push_back(t);
    delete b;
    return a;
}

Expr* exprSub(Expr* a, Expr* b) {
    a->constValue -= b->constValue;
    for (auto& t : b->terms)
        a->terms.push_back({ -t.first, t.second });  // flip b's signs
    delete b;
    return a;
}

bool exprEval(Expr* e, int& result) {
    const int SYMB_ABS = -1;
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    int value = e->constValue;
    for (auto& term : e->terms) {
        if (!symtab->isDefined(term.second)) { 
            return false;
        }
        value += term.first * getSymbolValue(term.second.c_str());
    }
    result = value;
    return true;
}

EquKind classifyEqu(Expr* e, int& value) {
    SymbolTable* symtab = ObjectFile::getSymbolTable();
    value = e->constValue;
    std::map<int, int> refs;          
    for (auto& term : e->terms) {
        if (!symtab->isDefined(term.second))
            return EQU_DEFER;

        value += term.first * getSymbolValue(term.second.c_str());

        if (!symtab->isAbsolute(term.second))
            refs[symtab->getSymbolSection(term.second)] += term.first;
    }

    int valid = true;
    for (auto& ref : refs)
        valid &= (ref.second == 0);
    return (valid ? EQU_ABSOLUTE : EQU_ERROR); 
}