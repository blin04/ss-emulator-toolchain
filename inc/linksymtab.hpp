#ifndef _LINK_SYMBOL_TABLE_H_
#define _LINK_SYMBOL_TABLE_H_

#include <fstream>
#include <map>
#include <unordered_set>
#include <string>
#include <vector>

#include "linkfile.hpp"
#include "outsection.hpp"
#include "symtab.hpp"

// Cross-file symbol bookkeeping, shared by both linker modes.
//
// registerFile() is called once per parsed LinkFile and is
// mode-agnostic (duplicate-definition checking applies either way).
//
// resolveFinal()/finalValue() are used only in full-link mode, once
// every OutputSection has a base address.
//
// mergedSymbolIndex()/mergedSymbols() are used only in relocatable
// mode, to renumber symbols into one merged table without computing
// any final address.
class GlobalSymbolTable {
public:
    struct MergedSymbol {
        int                     index;
        std::string             name;
        int                     section;
        int                     value;
        SymbolTable::SymbolBind bind;
        bool                    defined;
    };

    void registerFile(int fileIndex, const LinkFile& file);
    void addEntry(std::string name, int section, int value, SymbolTable::SymbolBind bind, bool defined = true);
    int getSymbolIndex(std::string name);

    // full-link mode
    void resolveFinal();
    int finalValue(int fileIndex, int localSymbolIndex);

    // relocatable mode
    void assignMergedIndices();
    int mergedSymbolIndex(int fileIndex, int localSymbolIndex);
    std::vector<MergedSymbol> mergedSymbols();

    void serialize(std::ofstream& out);

private:
    static int symbolIndex;
    // todo: per-file local symbol tables (never merged into a single
    // name->entry map up front - only GLOB names participate in
    // cross-file lookup, see plan notes)
    std::map<std::string, uint32_t> finalSymbolValues;
    // std::vector<const LinkFile&> files;
    std::map<LinkFile::LocalSymbol, std::pair<int, int>> foundSymbols;            // pairs of (fileIndex, symbolIndex)
    std::unordered_set<std::string> undefinedSymbols;
    std::map<std::string, MergedSymbol> symbols;
};

#endif
