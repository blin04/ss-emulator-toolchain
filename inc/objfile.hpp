#ifndef _OBJECT_FILE_H_
#define _OBJECT_FILE_H_

#include <vector>

class Section;
class SymbolTable;

class ObjectFile {
public:
    ~ObjectFile();

    void generate();
    void newSection(std::string name);
    void printSymbolTable();

    static Section* getCurrentSection();
    static SymbolTable* getSymbolTable();
    static ObjectFile* getInstance();
private:
    ObjectFile();

    Section* currentSection;
    std::vector<Section*> sections;
    SymbolTable* symbolTable;
};

#endif