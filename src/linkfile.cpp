#include <algorithm>
#include <iostream>
#include <iomanip>

#include "../inc/linkfile.hpp"

void LinkFile::printSection(RawSection section) {
    // write section contents to std::coutput
    bool newline = false;
    std::cout << "#" << section.name << "\n";
    int size = section.bytes.size();
    for (int i = 0; i < size; i += 4) {
        std::cout << std::hex << std::setw(2) << (int)section.bytes[i] << " ";
        
        if (i + 1 < size)
            std::cout << std::hex << std::setw(2) << (int)section.bytes[i + 1] << " ";
        if (i + 2 < size)
            std::cout << std::hex << std::setw(2) << (int)section.bytes[i + 2] << " ";
        if (i + 3 < size)
            std::cout << std::hex << std::setw(2) << (int)section.bytes[i + 3] << " ";

        if (newline) std::cout << "\n";
        else std::cout << "    ";

        newline = !newline;
    }

    if (newline) {
        std::cout << "\n";
        newline = false;
    }

    /* here, literal pool is merged with the contents of the section
    // write literal pool contents to std::coutput
    std::cout << "#." << section.name << ".litpool\n";
    for (int i = 0; i < litpool_bytes.size(); i += 4) {
        std::cout << std::hex << std::setw(2) << (int)litpool_bytes[i];
        std::cout << " ";
        std::cout << std::hex << std::setw(2) << (int)litpool_bytes[i + 1];
        std::cout << " ";
        std::cout << std::hex << std::setw(2) << (int)litpool_bytes[i + 2];
        std::cout << " ";
        std::cout << std::hex << std::setw(2) << (int)litpool_bytes[i + 3];
        if (newline) std::cout << "\n";
        else std::cout << "    ";
        newline = !newline;
    }

    if (newline) {
        std::cout << "\n";
        newline = false;
    }
    */

    // write relocation entries to std::coutput
    const int offsetWidth = 6;
    const int typeWidth = 5;
    const int symbolWidth = 10;
    const int addendWidth = 10;

    std::cout << std::dec << "#" << section.name << ".rela\n";
    std::cout << std::left
        << std::setw(offsetWidth) << "Offset" << " | "
        << std::setw(typeWidth) << "Type" << " | "
        << std::setw(symbolWidth) << "Symbol" << " | "
        << std::setw(addendWidth) << "Addend" << "\n";

    for (RelocEntry rela : section.relas) {
        std::string type = "ABS";
        if (rela.type == REL) type = "REL";

        std::cout << std::left
            << std::setw(offsetWidth) << rela.offset << " | "
            << std::setw(typeWidth) << type << " | "
            << std::setw(symbolWidth) << rela.symbol << " | "
            << std::setw(addendWidth) << rela.addend << "\n";
    }
}

void LinkFile::printSymTab() {
    int nameWidth = 4;
    for (const auto& symb : symbols) {
        if (symb.name.size() > nameWidth) nameWidth = symb.name.size();
    }


    const int indexWidth = 6;
    const int sectionWidth = 10;
    const int valueWidth = 6;
    const int typeWidth = 5;
    const int definedWidth = 7;

    std::cout << "#.symtab\n";
    std::cout << std::left
        << std::setw(indexWidth) << "Index" << " | "
        << std::setw(nameWidth) << "Name" << " | "
        << std::setw(sectionWidth) << "Section ID" << " | "
        << std::setw(valueWidth) << "Value" << " | "
        << std::setw(typeWidth) << "Bind" << " | "
        << std::setw(definedWidth) << "Defined?" << "\n";

    std::vector<LocalSymbol> orderedSymbols;
    for (const auto& symb : symbols) {
        orderedSymbols.push_back(symb);
    }

    std::sort(orderedSymbols.begin(), orderedSymbols.end(),
        [](const LocalSymbol lhs, const LocalSymbol rhs) {
            return lhs.index < rhs.index;
        });

    for (const auto symb : orderedSymbols) {
        std::string bind = "LOC";
        // if (symb->bind == SYMB_ABS) bind = "ABS";
        if (symb.bind == SymbolTable::SYMB_GLOB) bind = "GLOB";

        std::cout << std::left
            << std::setw(indexWidth) << symb.index << " | "
            << std::setw(nameWidth) << symb.name << " | ";

        if (symb.section != 0)
            std::cout << std::setw(sectionWidth) << symb.section;
        else
            std::cout << std::setw(sectionWidth) << "UND";
        std::cout << " | ";

        std::cout << std::setw(valueWidth) << symb.value << " | "
            << std::setw(typeWidth) << bind << " | "
            << std::setw(definedWidth) << (symb.defined ? "yes" : "no") << "\n";
    }
}

void LinkFile::print() {
    printSymTab();
    for (int i = 0; i < sections.size(); i++) {
        printSection(sections[i]);
    }
}

std::string LinkFile::getSectionName(int id) {
    return sections[id - 1].name;
}