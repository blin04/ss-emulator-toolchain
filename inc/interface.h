/*
*   Types and functions used as an interface for 
*   the bison parser to access & manipulate global 
*   objects written in C++
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <vector>
#include <string>

extern int location_counter;     // address (offset) inside a section

// Types

struct Operand {
    bool            fromMemory;
    int             gpr;
    unsigned int    disp;
    // bool    absolute;
    char*           symbol;
};

struct Expr {
    int constValue = 0;
    std::vector<std::pair<int, std::string>> terms;
};

// Functions

void addAsciiDirective(const char* str);
void addSkipDirective(int bytes_count);
int addWordDirective(char** initializers);

void defineSymbol(const char* name, int value, bool equ_defined = false);
void defineEquSymbol(const char* name, Expr* expr);
void declareSymbolsGlobal(char** symbols);
void declareSymbolsExtern(char** symbols);
int getSymbolValue(const char* name);
bool isAbsolute(const char* symbol);
bool isDefined(const char* symbol);
bool isExtern(const char* symbol);
void startNewSection(const char* name, int offset);

// statement handlers return the number of machine instructions
// emitted, so the parser can advance the location counter correctly
int zeroOpStatementHandler(int stmt);
int oneOpStatementHandler(int stmt, int op);
int oneOpJumpStatementHandler(int stmt, Operand op);
int twoOpStatementHandler(int stmt, int gpr1, int gpr2);
int threeOpStatementHandler(int stmt, int gpr1, int gpr2, Operand op);

int memoryStatementHandler(int type, Operand op, int gpr);

// expr builders

Expr* exprLiteral(int v);
Expr* exprSymbol(const char* s);
Expr* exprAdd(Expr* a, Expr* b);
Expr* exprSub(Expr* a, Expr* b);

#endif