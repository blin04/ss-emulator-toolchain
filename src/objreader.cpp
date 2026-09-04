#include "../inc/objreader.hpp"

LinkFile ObjReader::parse(const std::string& path) {
    LinkFile file;
    file.sourceFilename = path;

    // todo: read #.symtab into file.symbols
    // todo: for each #.<section> / #.<section>.litpool / #.<section>.rela
    //       triple, build a LinkFile::RawSection and append it

    return file;
}
