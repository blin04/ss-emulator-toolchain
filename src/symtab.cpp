#include "../inc/symtab.hpp"

#include <iostream>

SymbolTable::~SymbolTable() {
    for (auto it = symbols.begin(); it != symbols.end(); it++) {
        delete it->second;
    }
}

void SymbolTable::serialize(std::ofstream& out) {}

void SymbolTable::defineSymbol(std::string name, int sectionId, int offset, SymbolType type) {
    if (isDefined(name)) {
        // if symbol is already defined it means that
        // it was mentioned in .global directive
        symbols[name]->section = sectionId;
        symbols[name]->offset = offset;
    }
    else {
        Entry* e = new Entry();
        e->name = name;
        e->section = sectionId;
        e->offset = offset;
        e->type = type;
        symbols[name] = e;
    }
}

bool SymbolTable::isDefined(std::string symbol) { 
    if (symbols.count(symbol)) 
        return symbols[symbol]->type != SYMB_UND; 
    return false;
}

bool SymbolTable::isExtern(std::string symbol) {
    if (symbols.count(symbol))
        return symbols[symbol]->type == SYMB_UND;
    return false;
}

void SymbolTable::declareSymbolGlobal(std::string name) { 
    if (!isDefined(name)) defineSymbol(name, 0, 0, SYMB_GLOB);
    else symbols[name]->type = SYMB_GLOB; 
}

void SymbolTable::declareSymbolExtern(std::string name) { defineSymbol(name, 0, 0, SYMB_UND); }

int SymbolTable::getSymbolValue(std::string symbol) { 
    // if there is a request to get the value
    // of a symbol that still hasn't been defined,
    // then 0 should be returned and a relocation 
    // entry must be generated
    return symbols[symbol]->offset; 
}

void SymbolTable::print() {
    std::cout << "\tName | Section ID | Offset | Type\n";

    for (auto symb : symbols) {
        std::cout << '\t' << symb.second->name << " | " << symb.second->section << " | " << symb.second->offset << " | ";
        if (symb.second->type == SYMB_ABS) std::cout << "ABS\n";
        else if (symb.second->type == SYMB_GLOB) std::cout << "GLOB\n";
        else if (symb.second->type == SYMB_LOC) std::cout << "LOC\n";
        else std::cout << "UND\n";
    }
}