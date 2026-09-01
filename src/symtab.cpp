#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

int SymbolTable::symbol_index = 1;

SymbolTable::~SymbolTable() {
    for (auto it = symbols.begin(); it != symbols.end(); it++) {
        delete it->second;
    }
}

void SymbolTable::addEntry(std::string name, int sectionId, int value, SymbolBind bind, bool defined, bool equ) {
    Entry* e = new Entry();
    e->index = symbol_index++;
    e->name = name;
    e->section = sectionId;
    e->value = value;
    e->bind = bind;
    e->defined = defined;
    e->equ = equ;
    symbols[name] = e;
}

void SymbolTable::defineSymbol(std::string name, int sectionId, int offset, SymbolBind bind, bool equ) {
    if (symbols.count(name)) {
        // if symbol is already present in the table it means
        // that it was mentioned in a directive or statement
        symbols[name]->section = sectionId;
        symbols[name]->value = offset;
        symbols[name]->defined = true;
        symbols[name]->equ = equ;
    }
    else addEntry(name, sectionId, offset, bind, true, equ);
}

// symbol is defined if it was defined in code 
// either as a label or with .equ directive
// note: the value of defined symbol may not be final
bool SymbolTable::isDefined(std::string symbol) { 
    if (symbols.count(symbol))
        return symbols[symbol]->defined;
    return false;
}

// symbol is absolute if it was
// defined with .equ directive
bool SymbolTable::isAbsolute(std::string symbol) {
    if (symbols.count(symbol))
        return symbols[symbol]->equ;
    return false;
}

bool SymbolTable::isExtern(std::string symbol) {
    if (symbols.count(symbol))
        return symbols[symbol]->section == SYMB_UND;
    return false;
}

void SymbolTable::declareSymbolGlobal(std::string name) { 
    if (symbols.count(name) == 0) 
        addEntry(name, SYMB_UND, 0, SYMB_GLOB, false, false);
    else symbols[name]->bind = SYMB_GLOB; 
}

void SymbolTable::declareSymbolExtern(std::string name) { 
    if (symbols.count(name) == 0) 
        addEntry(name, 0, SYMB_UND, SYMB_GLOB, false, false); 
    else symbols[name]->section = SYMB_UND; 
}

int SymbolTable::getSymbolValue(std::string symbol) { 
    // if there is a request to get the value
    // of a symbol that still hasn't been defined,
    // then 0 should be returned and an entry 
    // with field `defined` set to false should
    // be generated 
    // the symbol is marked LOC, but this can
    // be changed to UND during backpatching phase
    // if no definition of the symbol was found
    // in the file that was assembled
    //
    // entry to forward reference table is added
    // during instruction creation 

    if (symbols.count(symbol) == 0)
        addEntry(
            symbol, 
            ObjectFile::getCurrentSection()->getSectionID(), 
            0, 
            SYMB_LOC, 
            false,
            false
        );
    return symbols[symbol]->value; 
}

int SymbolTable::getSymbolIndex(std::string symbol) {
    if (symbols.count(symbol)) 
        return symbols[symbol]->index;
    return -1;
}

SymbolTable::SymbolBind SymbolTable::getSymbolBind(std::string symbol) {
    if (symbols.count(symbol))
        return symbols[symbol]->bind;
    // not the best logic but will work
    return SYMB_LOC;
}

void SymbolTable::serialize(std::ostream& out) {
    int nameWidth = 4;
    for (const auto& symb : symbols) {
        if (symb.second->name.size() > nameWidth) nameWidth = symb.second->name.size();
    }


    const int indexWidth = 6;
    const int sectionWidth = 10;
    const int valueWidth = 6;
    const int typeWidth = 5;
    const int definedWidth = 7;

    out << "#.symtab\n";
    out << std::left
        << std::setw(indexWidth) << "Index" << " | "
        << std::setw(nameWidth) << "Name" << " | "
        << std::setw(sectionWidth) << "Section ID" << " | "
        << std::setw(valueWidth) << "Value" << " | "
        << std::setw(typeWidth) << "Bind" << " | "
        << std::setw(definedWidth) << "Defined?" << "\n";

    std::vector<const Entry*> orderedSymbols;
    for (const auto& symb : symbols) {
        orderedSymbols.push_back(symb.second);
    }

    std::sort(orderedSymbols.begin(), orderedSymbols.end(),
        [](const Entry* lhs, const Entry* rhs) {
            return lhs->index < rhs->index;
        });

    for (const auto* symb : orderedSymbols) {
        std::string bind = "LOC";
        // if (symb->bind == SYMB_ABS) bind = "ABS";
        if (symb->bind == SYMB_GLOB) bind = "GLOB";

        out << std::left
            << std::setw(indexWidth) << symb->index << " | "
            << std::setw(nameWidth) << symb->name << " | ";

        if (symb->section != SYMB_UND)
            out << std::setw(sectionWidth) << symb->section;
        else
            out << std::setw(sectionWidth) << "UND";
        out << " | ";

        out << std::setw(valueWidth) << symb->value << " | "
            << std::setw(typeWidth) << bind << " | "
            << std::setw(definedWidth) << (symb->defined ? "yes" : "no") << "\n";
    }
}