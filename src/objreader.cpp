#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../inc/objreader.hpp"

// cursor over a file's lines, used as a helper
// for easier parsing of object file 
struct LineCursor {
    std::vector<std::string> lines;
    size_t pos = 0;

    LineCursor(const std::string& path) {
        std::ifstream source(path);
        if (!source.is_open())
            throw std::runtime_error("could not open object file: " + path);

        std::string line;
        while (std::getline(source, line))
            lines.push_back(line);
    }

    bool atEnd() const { return pos >= lines.size(); }

    bool atBlockBoundary() const {
        return atEnd() || lines[pos].rfind("#.", 0) == 0;
    }

    std::string next() {
        // todo: throw if atEnd() is true, caller error
        return lines[pos++];
    }
};

// remove whitespaces around string
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

std::string trimBytesLine(const std::string& s) {
    bool inSpace = false;
    std::string result = "";
    for (char c : s) {
        if (c == ' ' || c == '\t') {
            inSpace = true;
            continue;
        }
        if (inSpace && !result.empty()) {
            result += ' ';          // keep only one whitespace
        }
        inSpace = false;
        result += c;
    }
    return result;
}

// splits a "field | field | field" row into trimmed tokens
std::vector<std::string> splitFields(const std::string& line, char delim) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, delim))
        fields.push_back(trim(field));
    return fields;
}

std::vector<LinkFile::LocalSymbol> parseSymtab(LineCursor& cur) {
    cur.next();             // consume header

    // line format:
    // Index | Name | Section ID | Value | Bind | Defined?

    std::string line;
    std::vector<LinkFile::LocalSymbol> symtab;
    while (!cur.atBlockBoundary()) {
        line = cur.next();
        LinkFile::LocalSymbol symb;

        std::vector<std::string> fields = splitFields(line, '|');

        symb.index   = std::stoi(fields[0]);
        symb.name    = fields[1];
        symb.section = (fields[2] == "UND") ? 0 : std::stoi(fields[2]);
        symb.value   = std::stoi(fields[3]);
        symb.bind    = (fields[4] == "GLOB") ? SymbolTable::SYMB_GLOB : SymbolTable::SYMB_LOC;
        symb.defined = (fields[5] == "yes");

        symtab.push_back(symb);
    }

    return symtab;
}

std::vector<uint8_t> parseBytes(LineCursor& cur) {
    std::string line;
    std::vector<uint8_t> bytes;
    while (!cur.atBlockBoundary()) {
        line = trimBytesLine(cur.next());
        std::vector<std::string> line_bytes = splitFields(line, ' '); 

        for (auto byte : line_bytes) {
            bytes.push_back(std::stoi(byte, nullptr, 16));
        }

    }
    return bytes;
}

std::vector<uint8_t> parseLitpool(LineCursor& cur) {
    std::vector<uint8_t> bytes;
    // todo: same shape as parseSection - litpool rows use the same
    // hex byte-dump format
    return bytes;
}

std::vector<LinkFile::RelocEntry> parseRela(LineCursor& cur) {
    cur.next();                     // consume header
    std::vector<LinkFile::RelocEntry> relas;
    std::string line;
    while (!cur.atBlockBoundary()) {
        line = cur.next();
        std::vector<std::string> fields = splitFields(line, '|'); 

        LinkFile::RelocEntry rel;
        rel.offset = std::stoi(fields[0]);
        rel.type = (fields[1] == "ABS") ? RelocType::ABS : RelocType::REL; 
        rel.symbol = std::stoi(fields[2]);
        rel.addend = std::stoi(fields[3]);

        relas.push_back(rel);
    }
    return relas;
}

LinkFile ObjReader::parse(const std::string& path) {
    LinkFile file;
    file.sourceFilename = path;

    LineCursor cur(path);

    cur.next();                       // consume "#.symtab" marker
    file.symbols = parseSymtab(cur);

    while (!cur.atEnd()) {
        std::string sectionMarker = cur.next();     // "#.<name>"
        std::string name = sectionMarker.substr(1);

        std::vector<uint8_t> content = parseBytes(cur);

        cur.next();                                  // "#.<name>.litpool"
        std::vector<uint8_t> pool = parseBytes(cur);
        content.insert(content.end(), pool.begin(), pool.end());

        cur.next();                                  // "#.<name>.rela"
        std::vector<LinkFile::RelocEntry> relas = parseRela(cur);

        file.sections.push_back({name, content, relas});
    }

    return file;
}
