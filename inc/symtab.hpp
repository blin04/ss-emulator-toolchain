#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <fstream>
#include <map>

class SymbolTable {
public:
    enum SymbolType {
        SYMB_GLOB,
        SYMB_LOC,
        SYMB_UND,      // extern
        SYMB_ABS        // .equ defined
    };

    ~SymbolTable();

    void addForwardReference();
    void backpatch();
    void defineSymbol(std::string name, int sectionId, int offset, SymbolType type);
    void declareSymbolGlobal(std::string symbol);
    void declareSymbolExtern(std::string symbol);
    int getSymbolValue(std::string symbol);
    bool isDefined(std::string symbol);
    void print();
    void serialize(std::ofstream& out);
private:
    typedef struct {
        std::string name;
        int         section;
        int         offset;
        SymbolType  type;
    } Entry;

    std::map<std::string, Entry*> symbols;
};

#endif