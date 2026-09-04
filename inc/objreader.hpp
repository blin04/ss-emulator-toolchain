#ifndef _OBJ_READER_H_
#define _OBJ_READER_H_

#include <string>

#include "linkfile.hpp"

// Parses a single assembler-produced object file (the #.symtab /
// #.<section> / #.<section>.litpool / #.<section>.rela text format
// written by SymbolTable::serialize / Section::serialize) back into
// a LinkFile. The inverse operation of those two serialize methods.
class ObjReader {
public:
    static LinkFile parse(const std::string& path);

private:
    // todo: split into parseSymtab(), parseSection(), parseLitpool(),
    // parseRela() helpers, one per '#.' block in the file
};

#endif
