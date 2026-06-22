#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <fstream>
#include <map>

class SymbolTable {
public:
    ~SymbolTable();

    void addForwardReference();
    void backpatch();
    void defineSymbol(std::string name, int sectionId, int offset, bool global);
    void declareSymbolGlobal(std::string symbol);
    void declareSymbolExtern(std::string symbol);
    int getSymbolValue(std::string symbol);
    bool isDefined(std::string symbol);
    void serialize(std::ofstream& out);
private:

    typedef struct {
        std::string name;
        int         section;
        int         offset;
        bool        global;
    } Entry;

    std::map<std::string, Entry*> symbols;

};

#endif