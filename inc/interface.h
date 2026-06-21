/*
*   Types and functions used as an interface for 
*   the bison parser to access & manipulate global 
*   objects written in C++
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

// Types

// Functions

void defineSymbol();
void declareSymbolGlobal();
void declareSymbolExtern();
void startNewSection();
void addInstruction();
void addDirective();
void addSkipDirective(int bytes_count);

#endif