#include <algorithm>
#include <iomanip>

#include "../inc/linker.hpp"
#include "../inc/objreader.hpp"
#include "../inc/linkfile.hpp"

// writes bytes in the same grouping the assembler uses: 4-byte groups
// space-separated, two groups per line separated by "    "
static void writeByteDump(std::ofstream& out, const std::vector<uint8_t>& bytes) {
    bool newline = false;
    int size = bytes.size();
    for (int i = 0; i < size; i += 4) {
        out << std::hex << std::setw(2) << (int)bytes[i] << " ";
        if (i + 1 < size) out << std::hex << std::setw(2) << (int)bytes[i + 1] << " ";
        if (i + 2 < size) out << std::hex << std::setw(2) << (int)bytes[i + 2] << " ";
        if (i + 3 < size) out << std::hex << std::setw(2) << (int)bytes[i + 3] << " ";

        out << (newline ? "\n" : "    ");
        newline = !newline;
    }
    if (newline) out << "\n";
}

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

        printSectionsLayout();
    }
    catch (const std::runtime_error& e) {
        std::cout << "error: " << e.what() << "\n";
    }
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
}

void Linker::buildSymbolTable() {
    // todo: symtab.registerFile() for every parsed file
    for (int i = 0; i < files.size(); i++) {
        symtab.registerFile(i, files[i]);
    }
}

// calculates base addresses of all output
// sections, taking into account the explicit 
// placements passed as CL arguments
// 
// sections without explicit placement are 
// placed one after the other, starting
// from the highest free memory address
void Linker::placeSections() {

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

    for (int i = 0; i < outputSections.size(); i++) {
        OutputSection* out_sec = outputSections[i];
        uint32_t size = out_sec->bytes.size();
        out_sec->baseAddress = highest_address;
        highest_address += size; 
    }

    validateSectionLayout();
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

void Linker::applyRelocations() {
    symtab.resolveFinal();

    OutputSection* out_sec;
    for (int i = 0; i < files.size(); i++) {
        for (LinkFile::RawSection& sec : files[i].sections) {
            out_sec = getOutputSection(sec.name);
            for (LinkFile::RelocEntry& rel : sec.relas) {
                int offset = out_sec->fileOffsets[i] + rel.offset;
                int value = symtab.finalValue(i, rel.symbol) + rel.addend;
                out_sec->writeWord(offset, value);
            }
        }
    }
}

void Linker::emitHexDump() {
    std::vector<OutputSection*> orderedOutputSections = outputSections;

    sort(orderedOutputSections.begin(), orderedOutputSections.end(), 
        [](const OutputSection* lhs, const OutputSection* rhs) {
            return lhs->baseAddress < rhs->baseAddress;
        });

    std::ofstream out(outputPath);
    for (OutputSection* out_sec : orderedOutputSections) {
        uint32_t addr = out_sec->baseAddress;
        for (int i = 0; i < out_sec->bytes.size(); i += 8, addr += 8) {
            out << std::hex << addr << ": ";

            out << std::hex << std::setw(2) << (int)out_sec->bytes[i] << " ";
            if (i + 1 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 1] << " ";
            if (i + 2 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 2] << " ";
            if (i + 3 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 3] << " ";
            if (i + 4 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 4] << " ";
            if (i + 5 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 5] << " ";
            if (i + 6 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 6] << " ";
            if (i + 7 < out_sec->bytes.size())
                out << std::hex << std::setw(2) << (int)out_sec->bytes[i + 7] << " ";
            out << "\n";
        }
    }

    out.close();
}

void Linker::renumberSymbols() {
    // todo: assign merged indexes via symtab.mergedSymbolIndex() for
    // every symbol referenced or defined across all files
    int section_id = 1;
    for (OutputSection* out_sec : outputSections) {
        symtab.addEntry(out_sec->name, section_id++, 0, SymbolTable::SYMB_LOC);        
    }
    symtab.assignMergedIndices();
}

void Linker::rewriteRelocations() {
    // todo: rewrite each relocation's offset (via fileOffsets) and
    // symbolIndex (via symtab.mergedSymbolIndex())
    OutputSection* out_sec;
    for (int i = 0; i < files.size(); i++) {
        for (LinkFile::RawSection& sec : files[i].sections) {
            out_sec = getOutputSection(sec.name);
            for (LinkFile::RelocEntry& rel : sec.relas) {
                // rel.symbol is +1 ??? check this!
                std::string symbol_name = files[i].symbols[rel.symbol - 1].name;
                int global_index = symtab.getSymbolIndex(symbol_name);
                int offset = out_sec->fileOffsets[i] + rel.offset;

                LinkFile::RelocEntry entry;
                entry.symbol = global_index;
                entry.offset = offset; 
                entry.addend = rel.addend;
                out_sec->relas.push_back(entry);
            }
        }
    }
}

void Linker::emitObjectFile() {
    std::ofstream out(outputPath);

    // ---- #.symtab ----
    symtab.serialize(out);

    const int offsetWidth = 6, relaTypeWidth = 5, symbolWidth = 10, addendWidth = 10;
    for (OutputSection* sec : outputSections) {
        out << "#" << sec->name << "\n";
        writeByteDump(out, sec->bytes);

        out << "#" << sec->name << ".litpool\n";

        out << std::dec << "#." << sec->name << ".rela\n";
        out << std::left
            << std::setw(offsetWidth)   << "Offset" << " | "
            << std::setw(relaTypeWidth) << "Type" << " | "
            << std::setw(symbolWidth)   << "Symbol" << " | "
            << std::setw(addendWidth)   << "Addend" << "\n";

        for (const LinkFile::RelocEntry& rel : sec->relas) {
            std::string type = (rel.type == REL) ? "REL" : "ABS";
            out << std::left
                << std::setw(offsetWidth)   << rel.offset << " | "
                << std::setw(relaTypeWidth) << type << " | "
                << std::setw(symbolWidth)   << rel.symbol << " | "
                << std::setw(addendWidth)   << rel.addend << "\n";
        }
    }

    out.close();
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