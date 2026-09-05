#include <iomanip>
#include <iostream>

#include "../inc/linksymtab.hpp"
#include "../inc/linkfile.hpp"
#include "../inc/linker.hpp"

void GlobalSymbolTable::registerFile(int fileIndex, const LinkFile& file) {
    // todo: register every GLOB-bind symbol from `file`; error on
    // duplicate *defined* globals across files
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

        // fileSymbols.push_back(file.symbols);
        // files.push_back(file);
    }
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
        const LinkFile& file = linker->files[file_index];
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

void GlobalSymbolTable::print() {
    // names already known to be defined - used so an extern reference
    // that turned out to be resolved elsewhere isn't printed twice
    std::unordered_set<std::string> foundNames;
    for (const auto& found : foundSymbols)
        foundNames.insert(found.first.name);

    int nameWidth = 4;
    for (const std::string& name : foundNames)
        if ((int)name.size() > nameWidth) nameWidth = name.size();
    for (const std::string& name : undefinedSymbols)
        if ((int)name.size() > nameWidth) nameWidth = name.size();

    const int indexWidth   = 6;
    const int sectionWidth = 10;
    const int valueWidth   = 6;
    const int typeWidth    = 5;
    const int definedWidth = 7;

    std::cout << "#.symtab\n";
    std::cout << std::left
        << std::setw(indexWidth)   << "Index" << " | "
        << std::setw(nameWidth)    << "Name" << " | "
        << std::setw(sectionWidth) << "Section ID" << " | "
        << std::setw(valueWidth)   << "Value" << " | "
        << std::setw(typeWidth)    << "Bind" << " | "
        << std::setw(definedWidth) << "Defined?" << "\n";

    int index = 1;
    for (const auto& found : foundSymbols) {
        const LinkFile::LocalSymbol& symbol = found.first;
        uint32_t value = finalSymbolValues.count(symbol.name)
            ? finalSymbolValues.at(symbol.name)
            : symbol.value;

        std::cout << std::left
            << std::setw(indexWidth)   << index++ << " | "
            << std::setw(nameWidth)    << symbol.name << " | "
            << std::setw(sectionWidth) << symbol.section << " | "
            << std::setw(valueWidth)   << std::hex << value << " | "
            << std::setw(typeWidth)    << "GLOB" << " | "
            << std::setw(definedWidth) << "yes" << "\n";
        std::cout << std::dec;
    }

    for (const std::string& name : undefinedSymbols) {
        if (foundNames.count(name))
            continue;   // resolved elsewhere - already printed above

        std::cout << std::left
            << std::setw(indexWidth)   << index++ << " | "
            << std::setw(nameWidth)    << name << " | "
            << std::setw(sectionWidth) << "UND" << " | "
            << std::setw(valueWidth)   << 0 << " | "
            << std::setw(typeWidth)    << "GLOB" << " | "
            << std::setw(definedWidth) << "no" << "\n";
    }
}