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

    void generate();
    void newSection(std::string name, int offset);
    void setOutput(std::string path);

    static Section* getCurrentSection();
    static SymbolTable* getSymbolTable();
    static ObjectFile* getInstance();
    static std::string getSectionFromID(int id);
private:
    ObjectFile();

    Section*                    currentSection;
    std::vector<Section*>       sections;
    SymbolTable*                symbolTable;
    std::string                 outputPath;
};

#endif