#include <iostream>

#include "../inc/interface.h"

void defineSymbol() {
    std::cout << "called defineSymbol()\n";
}

void declareSymbolGlobal() {
    std::cout << "called declareSymbolGlobal()\n";
}

void declareSymbolExtern() {
    std::cout << "called declareSymbolExtern()\n";
}

void newSection() {
    std::cout << "called newSection()\n";
}

void addInstruction() {
    std::cout << "called addInstruction()\n";
}

void addDirective() {
    std::cout << "called addDirective()\n";
}

void addSkipDirective(int bytes_count) {
    std::cout << "adding skip directive that skips " << bytes_count << " bytes\n";
}