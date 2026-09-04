#ifndef _OBJ_READER_H_
#define _OBJ_READER_H_

#include <string>

#include "linkfile.hpp"

// Parses a single assembler-produced object file (the #.symtab /
// #.<section> / #.<section>.litpool / #.<section>.rela text format
// written by SymbolTable::serialize / Section::serialize) back into
// a LinkFile. The inverse operation of those two serialize methods.
//
// Everything else (LineCursor, per-block parse helpers) is an
// implementation detail confined to objreader.cpp - nothing besides
// this one entry point is part of the contract.
class ObjReader {
public:
    static LinkFile parse(const std::string& path);
};

#endif
