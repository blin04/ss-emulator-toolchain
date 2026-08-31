#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <fstream>
#include <iostream>
#include <ostream>
#include <map>

class SymbolTable {
public:
    enum SymbolBind {
        SYMB_GLOB,
        SYMB_LOC,
    };

    ~SymbolTable();

    void defineSymbol(std::string name, int sectionId, int offset, SymbolBind bind, bool equ = false);
    void declareSymbolGlobal(std::string symbol);
    void declareSymbolExtern(std::string symbol);
    int getSymbolValue(std::string symbol);
    int getSymbolIndex(std::string symbol);
    SymbolBind getSymbolBind(std::string symbol);
    bool isAbsolute(std::string symbol);
    bool isDefined(std::string symbol);
    bool isExtern(std::string symbol);
    void serialize(std::ostream& out);
private:
    const int SYMB_UND = 0;

    typedef struct {
        int         index;
        std::string name;
        int         section;
        int         value;          // SYMB_LOC: offset in bytes from section start
        SymbolBind  bind;
        bool        equ;            // equ defined 
        bool        defined;        // says if the symbol's value is known
    } Entry;

    std::map<std::string, Entry*> symbols;

    void addEntry(std::string name, int section, int offset, SymbolBind type, bool defined, bool equ);

    static int symbol_index;
};

#endif