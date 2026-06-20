#ifndef _OBJECT_FILE_H_
#define _OBJECT_FILE_H_

#include <vector>

class Section;
class SymbolTable;

class ObjectFile {
public:
    void addSection(Section* s);
    void generate();
private:
    std::vector<Section*> sections;
    SymbolTable* symbolTable;
};

#endif