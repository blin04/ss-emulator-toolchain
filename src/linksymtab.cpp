#include "../inc/linksymtab.hpp"
#include "../inc/linkfile.hpp"

void GlobalSymbolTable::registerFile(int fileIndex, const LinkFile& file) {
    // todo: register every GLOB-bind symbol from `file`; error on
    // duplicate *defined* globals across files
    for (const LinkFile::LocalSymbol& symbol : file.symbols) {
        // linker ignores local symbols
        if (symbol.bind == SymbolTable::SYMB_LOC)
            continue;
        
        if (symbol.defined) {
            if (foundSymbols.count(symbol.name) != 0)
                throw std::runtime_error("found multiple definitions of symbol `" + symbol.name + "`");
            foundSymbols[symbol.name] = {fileIndex, symbol.index};
        }
        else undefinedSymbols.insert(symbol.name); 
    }
}

void GlobalSymbolTable::resolveFinal(const std::map<std::string, OutputSection>& sections) {
    // todo (full-link only): resolve every extern to its defining
    // file+value, compute final absolute address for every global
    // symbol using `sections`, error on anything still undefined
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
