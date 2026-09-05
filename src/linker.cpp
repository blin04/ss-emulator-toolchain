#include <algorithm>
#include <iomanip>

#include "../inc/linker.hpp"
#include "../inc/objreader.hpp"
#include "../inc/linkfile.hpp"

Linker::Linker() 
    : outputMode(OutputMode::UNSET) 
    , sectionIndex(0) {}

Linker::~Linker() {
    for (int i = 0; i < outputSections.size(); i++)
        delete outputSections[i];
}

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

    try {
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
    catch (const std::runtime_error& e) {
        std::cout << "error: " << e.what() << "\n";
    }

    // debug: printing files
    std::cout << "\n====================== parsed files ======================\n";
    for (LinkFile file : files) {
        std::cout << "\n";
        file.print();
        std::cout << "\n";
    }

    printSectionsLayout();
}

void Linker::parseInputs() {
    // todo: ObjReader::parse() every path in inputPaths into `files`
    LinkFile file;
    for (const std::string& path : inputPaths) {
        file = ObjReader::parse(path);
        files.push_back(file);
    }
}

void Linker::mergeSections() {
    // todo: for each unique section name (first-seen order), build an
    // OutputSection by concatenating every file's blob for that name;
    // record fileOffsets as you go

    std::cout << "called maderfaker\n";

    for (int i = 0; i < files.size(); i++) {
        LinkFile& file = files[i];
        for (LinkFile::RawSection& sec : file.sections) {
            OutputSection* out_sec;
            if (outputSectionToIndex.count(sec.name) == 0) {
                out_sec = new OutputSection();
                out_sec->name = sec.name;
                outputSectionToIndex[sec.name] = outputSections.size();
                outputSections.push_back(out_sec);
            }
            else out_sec = outputSections[outputSectionToIndex[sec.name]];

            out_sec->fileOffsets[i] = out_sec->bytes.size();
            out_sec->bytes.insert(
                out_sec->bytes.end(), sec.bytes.begin(), sec.bytes.end()
            );
        }
    }

    calculateSectionBases();
    validateSectionLayout();
}

void Linker::buildSymbolTable() {
    // todo: symtab.registerFile() for every parsed file
}

// calculates base addresses of all output
// sections, taking into account the explicit 
// placements passed as CL arguments
// 
// sections without explicit placement are 
// placed one after the other, starting
// from the highest free memory address
void Linker::calculateSectionBases() {

    uint32_t highest_address = 0;
    std::vector<bool> handled(false, outputSections.size());
    for (auto placement : explicitPlacements) {
        int index = outputSectionToIndex[placement.first];
        OutputSection* out_sec = outputSections[index];
        uint32_t size = out_sec->bytes.size();

        out_sec->baseAddress = placement.second;
        if (placement.second + size > highest_address) 
            highest_address = placement.second + size;

        handled[index] = true;
    }

    highest_address++;          // start from free address
    for (int i = 0; i < outputSections.size(); i++) {
        OutputSection* out_sec = outputSections[i];
        uint32_t size = out_sec->bytes.size();
        out_sec->baseAddress = highest_address;
        highest_address += size; 
    }
}

// validates that there are no sections
// overlapping each other and that the 
// whole program fits into 32bit address
// space
void Linker::validateSectionLayout() {
    // pairs of (start, size)
    std::vector<std::pair<uint32_t, uint32_t>> start_addresses;
    for (int i = 0; i < outputSections.size(); i++) {
        start_addresses.push_back(
            {outputSections[i]->baseAddress, outputSections[i]->bytes.size()}
        );
    }

    std::sort(start_addresses.begin(), start_addresses.end());

    uint32_t marker = 0;
    for (int i = 0; i < start_addresses.size(); i++) {
        if (marker > start_addresses[i].first) {
            throw std::runtime_error("section overlap detected near "
                + std::to_string(start_addresses[i].first)
                + ", correct your explicit statements"
            );
        }
        marker = start_addresses[i].first + start_addresses[i].second;
    }
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

void Linker::printSectionsLayout() {
    std::cout << "\n====================== section layout ======================\n";
    std::cout << std::left  << std::setw(12) << "Name"
               << std::right << std::setw(12) << "Base"
               << std::setw(12) << "Size"
               << std::setw(12) << "End" << "\n";

    for (const OutputSection* sec : outputSections) {
        uint32_t size = sec->bytes.size();
        uint32_t end  = sec->baseAddress + size - 1;

        std::cout << std::left << std::setw(12) << sec->name
                   << std::right
                   << "0x" << std::hex << std::setfill('0') << std::setw(8) << sec->baseAddress
                   << std::dec << std::setfill(' ')
                   << std::setw(12) << size
                   << "  0x" << std::hex << std::setfill('0') << std::setw(8) << end
                   << std::dec << std::setfill(' ') << "\n";
    }
    std::cout << std::endl;
}