/*
*   Types and functions used as an interface for 
*   the bison parser to access & manipulate global 
*   objects written in C++
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

// Types

// Functions

void addDirective();
void addAsciiDirective(const char* str);
void addSkipDirective(int bytes_count);
int addWordDirective(char** initializers);

void defineSymbol(const char* name, int value, bool equ_defined = false);
void declareSymbolsGlobal(char** symbols);
void declareSymbolsExtern(char** symbols);
int getSymbolValue(const char* name);
bool isDefined(const char* symbol);
bool isExtern(const char* symbol);
void startNewSection(const char* name, int offset);

void haltHandler();
void intHandler();
void iretHandler();
void callHandler();
void retHandler();
void jmpHandler();
void beqHandler();
void bneHandler();
void bgtHandler();
void pushHandler();
void popHandler();
void xchgHandler();
void addHandler();
void subHandler();
void mulHandler();
void divHandler();
void notHandler();
void andHandler();
void orHandler();
void xorHandler();
void shlHandler();
void shrHandler();
void ldHandler();
void stHandler();
void csrrdHandler();
void csrwrHandler();

#endif