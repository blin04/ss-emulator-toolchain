#include "../inc/line.hpp"
#include "../inc/section.hpp"

Section::Section(std::string sectionName) : name(sectionName) {}

void Section::addLine(Line* line) {
    lines.push_back(line);
}

void Section::serialize(std::ofstream& out) {
    for (Line* l : lines) {
        std::vector<uint8_t> bytes = l->generateBytes();
        for (uint8_t byte : bytes) out << byte;
    }
}