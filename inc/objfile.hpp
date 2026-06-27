#ifndef _OBJECT_FILE_H_
#define _OBJECT_FILE_H_


#include <string>
#include <vector>

class ForwardReferenceTable;
class Section;
class SymbolTable;

class ObjectFile {
public:
    ~ObjectFile();

    void backpatch();
    void generate();
    void newSection(std::string name, int offset);
    void printSymbolTable();

    static Section* getCurrentSection();
    static SymbolTable* getSymbolTable();
    static ObjectFile* getInstance();
    static ForwardReferenceTable* getForwardReferenceTable();
private:
    ObjectFile();

    ForwardReferenceTable* freftab;
    Section* currentSection;
    std::vector<Section*> sections;
    SymbolTable* symbolTable;
};

#endif