#ifndef _OBJECT_FILE_H_
#define _OBJECT_FILE_H_


#include <string>
#include <utility>
#include <vector>

class ForwardReferenceTable;
class Section;
class SymbolTable;
struct Expr;

class ObjectFile {
public:
    ~ObjectFile();

    void generate();
    void newSection(std::string name, int offset);
    void setOutput(std::string path);

    void addPendingEqu(const char* name, Expr* expr);
    bool resolvePendingEqus();

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
    std::vector<std::pair<std::string, Expr*>> pendingEqus;
};

#endif