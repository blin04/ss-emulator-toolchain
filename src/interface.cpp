#include <iostream>

#include "../inc/directives.hpp"
#include "../inc/interface.h"
#include "../inc/line.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"

void defineSymbol() {
    std::cout << "called defineSymbol()\n";
}

void declareSymbolGlobal() {
    std::cout << "called declareSymbolGlobal()\n";
}

void declareSymbolExtern() {
    std::cout << "called declareSymbolExtern()\n";
}

void startNewSection(const char* name) {
    std::cout << "creating section named " << name << "\n";
    ObjectFile::getInstance()->newSection(name);
}

void addInstruction() {
    std::cout << "called addInstruction()\n";
}

void addDirective() {
    std::cout << "called addDirective()\n";
}

void addSkipDirective(int bytes_count) {
    std::cout << "adding skip directive that skips " << bytes_count << " bytes\n";
    Section* curr = ObjectFile::getCurrentSection();
    curr->addLine(new SkipDirective(bytes_count));
}

void addWordDirective(char** initializers) {
    Section* curr = ObjectFile::getCurrentSection();
    std::vector<std::string> initializers_param;

    for (int i = 0; initializers[i] != nullptr; i++) {
        initializers_param.push_back(initializers[i]);
        free(initializers[i]);
    }
    WordDirective* w = new WordDirective(initializers_param);
    curr->addLine(new WordDirective(initializers_param));
    free(initializers);
}
