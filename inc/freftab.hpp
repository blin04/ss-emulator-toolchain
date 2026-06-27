#ifndef _FORWARD_REFERENCE_TABLE_H_
#define _FORWARD_REFERENCE_TABLE_H_

#include <map>
#include <string>
#include <vector>

class ForwardReferenceTable {
public:
    void addEntry(std::string symbol, int offset);

    // debug purposes
    void print();
private:
    // symbol | offsets list
    std::map<std::string, std::vector<int>> entries;

    friend class ObjectFile;
};

#endif
