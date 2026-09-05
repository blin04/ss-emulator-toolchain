#ifndef _LINKER_H_
#define _LINKER_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "linkfile.hpp"
#include "outsection.hpp"
#include "linksymtab.hpp"

// Driver singleton for the linker, mirrors the role ObjectFile plays
// for the assembler. Owns the pass order described in the roadmap
// (see plan curious-marinating-bee): parse -> merge sections -> build
// symbol table, then fork into full-link (place -> resolve -> patch
// bytes -> hex dump) or relocatable (renumber symbols -> rewrite
// relocations -> re-serialize as a merged .o).
class Linker {
public:
    static Linker* getInstance();

    void addInputFile(const std::string& path);
    void makeRelocatable();
    void makeHex();
    void setOutputPath(const std::string& path);
    void setPlacement(const std::string& section, uint32_t address);
    void link();

private:

    enum OutputMode {
        UNSET,
        HEX,
        RELOC
    };

    Linker();
    ~Linker();

    // shared passes
    void parseInputs();
    void mergeSections();
    void buildSymbolTable();
    void calculateSectionBases();
    void validateSectionLayout();

    // full-link only
    void placeSections();
    void applyRelocations();
    void emitHexDump();

    // relocatable only
    void renumberSymbols();
    void rewriteRelocations();
    void emitObjectFile();

    // debug / testing
    void printSectionsLayout();

    std::map<std::string, uint32_t>         explicitPlacements;
    std::vector<LinkFile>                   files;
    std::vector<std::string>                inputPaths;
    OutputMode                              outputMode;
    std::string                             outputPath;
    std::vector<OutputSection*>             outputSections;
    std::unordered_map<std::string, int>    outputSectionToIndex;
    int                                     sectionIndex;
    GlobalSymbolTable                       symtab;
};

#endif
