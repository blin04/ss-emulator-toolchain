#ifndef _LINK_SYMBOL_TABLE_H_
#define _LINK_SYMBOL_TABLE_H_

#include <map>
#include <unordered_set>
#include <string>
#include <vector>

#include "linkfile.hpp"
#include "outsection.hpp"

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

    // full-link mode
    void resolveFinal(const std::map<std::string, OutputSection>& sections);
    int  finalValue(int fileIndex, int localSymbolIndex);

    // relocatable mode
    int mergedSymbolIndex(int fileIndex, int localSymbolIndex);
    std::vector<MergedSymbol> mergedSymbols();

private:
    // todo: per-file local symbol tables (never merged into a single
    // name->entry map up front - only GLOB names participate in
    // cross-file lookup, see plan notes)
    std::map<std::string, std::pair<int, int>> foundSymbols;
    std::unordered_set<std::string> undefinedSymbols;
};

#endif
