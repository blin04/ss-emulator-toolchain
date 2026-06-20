#include <fstream>

#include "../inc/objfile.hpp"
#include "../inc/section.hpp"
#include "../inc/symtab.hpp"

void ObjectFile::generate() {
    // file name hardcoded for now
    std::ofstream out("main.obj", std::ios::out | std::ios::binary);

    symbolTable->serialize(out);

    // todo: from sections vector extract
    // data needed for Section Table and
    // serialize it

    for (Section*& s : sections) {
        s->serialize(out);    
    }

    out.close();
}