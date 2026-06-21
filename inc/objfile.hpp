#ifndef _OBJECT_FILE_H_
#define _OBJECT_FILE_H_

#include <vector>

class Section;
class SymbolTable;

class ObjectFile {
public:
    void newSection(std::string name);
    void generate();

    static Section* getCurrentSection();
    static ObjectFile* getInstance();
private:
    ObjectFile();

    Section* currentSection;
    std::vector<Section*> sections;
    SymbolTable* symbolTable;
};

#endif