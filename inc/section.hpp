#ifndef _SECTION_H_
#define _SECTION_H_

#include <vector>

class Line;

class Section {
public:
    void addRelocation();
    void addLine(Line*);
    void serialize();
private:
    std::vector<Line*> lines;
};

#endif