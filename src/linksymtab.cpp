#include <algorithm>
#include <iomanip>
#include <iostream>

#include "../inc/linksymtab.hpp"
#include "../inc/linkfile.hpp"
#include "../inc/linker.hpp"

int GlobalSymbolTable::symbolIndex = 1;

void GlobalSymbolTable::registerFile(int fileIndex, const LinkFile& file) {
    for (const LinkFile::LocalSymbol& symbol : file.symbols) {
        // linker ignores local symbols
        if (symbol.bind == SymbolTable::SYMB_LOC)
            continue;
        
        if (symbol.defined) {
            if (foundSymbols.count(symbol) != 0)
                throw std::runtime_error("found multiple definitions of symbol `" + symbol.name + "`");
            foundSymbols[symbol] = {fileIndex, symbol.index};
        }
        else undefinedSymbols.insert(symbol.name); 
    }
}

void GlobalSymbolTable::addEntry(std::string name, int section, int value, SymbolTable::SymbolBind bind, bool defined) {
    MergedSymbol entry;
    entry.index = symbolIndex++;
    entry.name = name;
    entry.section = section;
    entry.value = value;
    entry.bind = bind;
    entry.defined = defined;
    symbols[name] = entry;
}

int GlobalSymbolTable::getSymbolIndex(std::string name) {
    if (symbols.count(name) == 0)
        return -1;
    return symbols[name].index;
}

void GlobalSymbolTable::resolveFinal() {
    // todo (full-link only): resolve every extern to its defining
    // file+value, compute final absolute address for every global
    // symbol using `sections`, error on anything still undefined

    // check for references to undefined symbols
    /* for (const std::string& name : undefinedSymbols) {
    if (foundSymbols.count(name) == 0)
        throw std::runtime_error("undefined symbol: " + name);
    } */

    /* OutputSection* out_sec;
    for (int file_index = 0; file_index < fileSymbols.size(); file_index++) {
        for (const LinkFile::LocalSymbol& symbol : fileSymbols[file_index]) {
            out_sec = 
            finalSymbolValues[symbol.name] = sectionBase + 

        }
    } */

    Linker* linker = Linker::getInstance();
    for (auto& found : foundSymbols) {
        LinkFile::LocalSymbol symbol = found.first;

        // remove references to this symbol
        if (undefinedSymbols.count(symbol.name)) {
            undefinedSymbols.erase(symbol.name);
        }

        int file_index = found.second.first;
        int symbol_index = found.second.second;
        LinkFile& file = linker->files[file_index];
        std::string section_name = file.sections[symbol.section - 1].name;
        OutputSection* out_sec = linker->getOutputSection(section_name);

        finalSymbolValues[symbol.name] = out_sec->baseAddress 
            + out_sec->fileOffsets[file_index] 
            + symbol.value; 
    }

    if (undefinedSymbols.size()) {
        std::string symbols = "";
        for (auto it = undefinedSymbols.begin(); it != undefinedSymbols.end(); it++) {
            if (it != undefinedSymbols.begin())
                symbols += ", ";
            symbols += *it;
        }
        throw std::runtime_error("undefined symbols: " + symbols);
    }

}

int GlobalSymbolTable::finalValue(int fileIndex, int localSymbolIndex) {
    // todo (full-link only)
    return 0;
}

// maps LocalSymbol entries to MergedSymbol
void GlobalSymbolTable::assignMergedIndices() {
    Linker* linker = Linker::getInstance();
    for (auto found : foundSymbols) {
        LinkFile::LocalSymbol symbol = found.first;
        int file_index = found.second.first;
        LinkFile& file = linker->files[file_index];
        std::string section_name = file.getSectionName(symbol.section);
        int section_index = getSymbolIndex(section_name);
        addEntry(symbol.name, section_index, symbol.value, symbol.bind);
    }
}

int GlobalSymbolTable::mergedSymbolIndex(int fileIndex, int localSymbolIndex) {
    // todo (relocatable only): locals get fresh sequential indexes,
    // matching globals collapse to one shared index, still-undefined
    // globals keep a merged index so relocations can still reference them
    return 0;
}

std::vector<GlobalSymbolTable::MergedSymbol> GlobalSymbolTable::mergedSymbols() {
    // todo (relocatable only)
    return {};
}

void GlobalSymbolTable::serialize(std::ofstream& out) {
    std::vector<const MergedSymbol*> ordered;
    for (const auto& entry : symbols)
        ordered.push_back(&entry.second);

    std::sort(ordered.begin(), ordered.end(),
        [](const MergedSymbol* a, const MergedSymbol* b) {
            return a->index < b->index;
        });

    int nameWidth = 4;
    for (const MergedSymbol* s : ordered)
        if ((int)s->name.size() > nameWidth) nameWidth = s->name.size();

    const int indexWidth   = 6;
    const int sectionWidth = 10;
    const int valueWidth   = 6;
    const int typeWidth    = 5;
    const int definedWidth = 7;

    out << "#.symtab\n";
    out << std::left
        << std::setw(indexWidth)   << "Index" << " | "
        << std::setw(nameWidth)    << "Name" << " | "
        << std::setw(sectionWidth) << "Section ID" << " | "
        << std::setw(valueWidth)   << "Value" << " | "
        << std::setw(typeWidth)    << "Bind" << " | "
        << std::setw(definedWidth) << "Defined?" << "\n";

    for (const MergedSymbol* s : ordered) {
        std::string bind = (s->bind == SymbolTable::SYMB_GLOB) ? "GLOB" : "LOC";

        out << std::left
            << std::setw(indexWidth) << s->index << " | "
            << std::setw(nameWidth)  << s->name << " | ";
        if (s->section != 0)
            out << std::setw(sectionWidth) << s->section;
        else
            out << std::setw(sectionWidth) << "UND";
        out << " | "
            << std::setw(valueWidth)   << s->value << " | "
            << std::setw(typeWidth)    << bind << " | "
            << std::setw(definedWidth) << (s->defined ? "yes" : "no") << "\n";
    }
}