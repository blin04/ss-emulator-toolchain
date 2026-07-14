/*
*   Types and functions used as an interface for 
*   the bison parser to access & manipulate global 
*   objects written in C++
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

extern int location_counter;     // address (offset) inside a section

// Types

typedef struct operand {
    bool    fromMemory;
    int     gpr;
    int     disp;
    // bool    absolute;
    char*   symbol;
} Operand;

// Functions

void addDirective();
void addAsciiDirective(const char* str);
void addSkipDirective(int bytes_count);
int addWordDirective(char** initializers);

void defineSymbol(const char* name, bool equ_defined = false);
void declareSymbolsGlobal(char** symbols);
void declareSymbolsExtern(char** symbols);
int getSymbolValue(const char* name);
bool isDefined(const char* symbol);
bool isExtern(const char* symbol);
void startNewSection(const char* name, int offset);

void zeroOpStatementHandler(int stmt);
void oneOpStatementHandler(int stmt, int op);
void oneOpJumpStatementHandler(int stmt, Operand op);
void twoOpStatementHandler(int stmt, int gpr1, int gpr2);
void threeOpStatementHandler(int stmt, int gpr1, int gpr2, Operand op);

void memoryStatementHandler(int type, Operand op, int gpr);

#endif