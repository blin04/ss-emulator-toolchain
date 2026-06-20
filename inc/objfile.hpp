#ifndef _OBJECT_FILE_H_
#define _OBJECT_FILE_H_

#include <vector>

class Section;

class ObjectFile {
public:
    void addSection(Section* s);
    void serialize();
private:
    std::vector<Section*> sections;
};

#endif