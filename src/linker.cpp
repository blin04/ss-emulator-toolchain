#include "../inc/linker.hpp"

Linker::Linker() 
    : outputMode(OutputMode::UNSET) {}

Linker* Linker::getInstance() {
    static Linker instance;
    return &instance;
}

void Linker::addInputFile(const std::string& path) {
    inputPaths.push_back(path);
}

void Linker::makeHex() { 
    outputMode = OutputMode::HEX;
}

void Linker::makeRelocatable() { 
    outputMode = OutputMode::RELOC;
}

void Linker::setOutputPath(const std::string& path) {
    outputPath = path;
}

void Linker::setPlacement(const std::string& section, uint32_t address) {
    explicitPlacements[section] = address;
}

void Linker::link() {

    // output mode must be specified
    if (outputMode == OutputMode::UNSET) {
        std::cout << "error: no output mode set, use `-hex` or `-relocatable`" << std::endl;
        return;
    }

    // debug case
    std::cout << "Constructed linker with input files: ";
    for (auto input : inputPaths) {
        std::cout << input;
        if (input != inputPaths.back())
            std::cout << ", ";
    }
    std::cout << "\n";

    parseInputs();
    mergeSections();
    buildSymbolTable();

    if (outputMode == OutputMode::RELOC) {
        renumberSymbols();
        rewriteRelocations();
        emitObjectFile();
    }
    else {
        placeSections();
        applyRelocations();
        emitHexDump();
    }
}

void Linker::parseInputs() {
    // todo: ObjReader::parse() every path in inputPaths into `files`
}

void Linker::mergeSections() {
    // todo: for each unique section name (first-seen order), build an
    // OutputSection by concatenating every file's blob for that name;
    // record fileOffsets as you go
}

void Linker::buildSymbolTable() {
    // todo: symtab.registerFile() for every parsed file
}

void Linker::placeSections() {
    // todo: assign baseAddress to every OutputSection - explicit
    // explicitPlacements entries first (error on overlap), then
    // auto-place the rest contiguously
}

void Linker::applyRelocations() {
    // todo: for every file/section/relocation, compute the global
    // offset and patched value via symtab.finalValue(), write 4 bytes
    // little-endian into the OutputSection's bytes
}

void Linker::emitHexDump() {
    // todo: walk outputSections sorted by baseAddress, print
    // contiguous "ADDR: b1 b2 ... b8" lines
}

void Linker::renumberSymbols() {
    // todo: assign merged indexes via symtab.mergedSymbolIndex() for
    // every symbol referenced or defined across all files
}

void Linker::rewriteRelocations() {
    // todo: rewrite each relocation's offset (via fileOffsets) and
    // symbolIndex (via symtab.mergedSymbolIndex())
}

void Linker::emitObjectFile() {
    // todo: serialize symtab.mergedSymbols() + each OutputSection's
    // bytes/litpool split back out + rewritten relocations, in the
    // assembler's own text format
}
