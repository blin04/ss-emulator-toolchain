/*
*   Types and functions used as an interface for 
*   the bison parser to access & manipulate global 
*   objects written in C++
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

// Types

// Functions

void addInstruction();
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
void startNewSection(const char* name);

#endif