#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

class SymbolTable {
public:
    void defineSymbol();
    bool isDefined();
    void addForwardReference();
    void backpatch();
};

#endif