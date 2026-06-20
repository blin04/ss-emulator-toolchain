#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <fstream>

class SymbolTable {
public:
    void addForwardReference();
    void backpatch();
    void defineSymbol();
    bool isDefined();
    void serialize(std::ofstream& out);
};

#endif