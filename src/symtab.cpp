#include "../inc/symtab.hpp"

#include <iostream>

SymbolTable::~SymbolTable() {
    for (auto it = symbols.begin(); it != symbols.end(); it++) {
        delete it->second;
    }
}

void SymbolTable::addEntry(std::string name, int sectionId, int offset, SymbolType type, bool defined) {
    Entry* e = new Entry();
    e->name = name;
    e->section = sectionId;
    e->offset = offset;
    e->type = type;
    e->defined = defined;
    symbols[name] = e;
}

void SymbolTable::serialize(std::ofstream& out) {}

void SymbolTable::defineSymbol(std::string name, int sectionId, int offset, SymbolType type) {
    if (symbols.count(name)) {
        // if symbol is already present in the table it means
        // that it was mentioned in a directive or statement
        symbols[name]->section = sectionId;
        symbols[name]->offset = offset;
        symbols[name]->defined = true;
    }
    else addEntry(name, sectionId, offset, type, true);
}

bool SymbolTable::isDefined(std::string symbol) { 
    if (symbols.count(symbol))
        return symbols[symbol]->defined;
    return false;
}

bool SymbolTable::isExtern(std::string symbol) {
    if (symbols.count(symbol))
        return symbols[symbol]->type == SYMB_UND;
    return false;
}

void SymbolTable::declareSymbolGlobal(std::string name) { 
    if (symbols.count(name) == 0) 
        addEntry(name, 0, 0, SYMB_GLOB, false);
    else symbols[name]->type = SYMB_GLOB; 
}

void SymbolTable::declareSymbolExtern(std::string name) { addEntry(name, 0, 0, SYMB_UND, false); }

int SymbolTable::getSymbolValue(std::string symbol) { 
    // if there is a request to get the value
    // of a symbol that still hasn't been defined,
    // then 0 should be returned and an entry 
    // with field `defined` set to false should
    // be generated 
    // the symbol is marked LOC, but this can
    // be changed to UND during backpatching phase
    // if no definition of the symbol was found
    //
    // entry to forward reference table is added
    // during instruction creation 

    if (symbols.count(symbol) == 0)
        addEntry(symbol, 0, 0, SYMB_LOC, false);
    return symbols[symbol]->offset; 
}

void SymbolTable::print() {
    std::cout << "\tName | Section ID | Offset | Type | Defined? \n";

    for (auto symb : symbols) {
        std::cout << '\t' << symb.second->name << " | " << symb.second->section << " | " << symb.second->offset << " | ";
        if (symb.second->type == SYMB_ABS) std::cout << "ABS";
        else if (symb.second->type == SYMB_GLOB) std::cout << "GLOB";
        else if (symb.second->type == SYMB_LOC) std::cout << "LOC";
        else std::cout << "UND";
        std::cout << " | " << (symb.second->defined ? "yes" : "no") << "\n";
    }
}