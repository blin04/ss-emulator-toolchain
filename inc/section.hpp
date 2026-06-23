#ifndef _SECTION_H_
#define _SECTION_H_

#include <vector>
#include <fstream>

class Line;
class RelocationTable;

class Section {
public:
    Section(std::string name, int offset);
    ~Section();

    void addRelocation();
    void addLine(Line*);
    int getSectionID();
    void serialize(std::ofstream& file);
private:
    int index;
    std::vector<Line*> lines;
    std::string name;
    int offset;
    int startAddress;
    RelocationTable* relocations; 

    static int counter;
};

#endif