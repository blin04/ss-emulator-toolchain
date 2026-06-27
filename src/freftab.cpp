#include <iostream>

#include "../inc/freftab.hpp"

void ForwardReferenceTable::addEntry(std::string symbol, int offset) {
    entries[symbol].push_back(offset);
}

void ForwardReferenceTable::print() {
    std::cout << "Symbol reference table " << "\n";
    for (auto& e : entries) {
        std::cout << "\t'" << e.first << "' forward referenced at offsets: \n";
        for (int i = 0; i < e.second.size(); i++) std::cout << e.second[i] << " ";
        std::cout << "\n";
    }
}