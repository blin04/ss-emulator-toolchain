#ifndef _SECTION_H_
#define _SECTION_H_

#include <vector>
#include <fstream>

class Line;
class RelocationTable;

class Section {
public:
    Section(std::string name);
    void addRelocation();
    void addLine(Line*);
    void serialize(std::ofstream& file);
private:
    std::string name;
    std::vector<Line*> lines;
    RelocationTable* relocations; 
};

#endif