#include "../inc/symtab.hpp"

SymbolTable::~SymbolTable() {
    for (auto it = symbols.begin(); it != symbols.end(); it++) {
        delete it->second;
    }
}

void SymbolTable::serialize(std::ofstream& out) {}

void SymbolTable::defineSymbol(std::string name, int sectionId, int offset, bool global) {
    Entry* e = new Entry();
    e->name = name;
    e->section = sectionId;
    e->offset = offset;
    e->global = global;
    symbols[name] = e;
}

bool SymbolTable::isDefined(std::string symbol) { return symbols.count(symbol); }

void SymbolTable::declareSymbolGlobal(std::string name) { symbols[name]->global = true; }

void SymbolTable::declareSymbolExtern(std::string name) { 
    /* add correct entry to symbol table */
}

int SymbolTable::getSymbolValue(std::string symbol) { /* what here ? */ }