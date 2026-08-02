#ifndef _SECTION_H_
#define _SECTION_H_

#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>

enum RelocType { ABS, REL };

class Line;

class Section {
public:
    Section(std::string name, int offset);
    ~Section();

    // returns index of the added relocation
    int addRelocation(int offset, RelocType type, std::string symbol, int addend);
    void addLine(Line*, bool fromPool = false);
    // returns index of the added value in the pool
    int addLiteralPoolValue(int value, const char* symbol);
    void backpatch();
    int getSectionID();
    void serialize(std::ofstream& file);
private:

    typedef struct {
        int         offset;
        RelocType   type;
        std::string symbol;
        int         addend;
    } RelocEntry;

    typedef struct {
        // if symbol is set it's addend, else
        // it's just a constant value
        int         value;
        // referenced symbol, might need relocation
        // empty if plain constant (literal) is
        // referenced
        std::string symbol;
    } LitPoolEntry;

    // list of indexes of instructions that access 
    // symbols in literal pool with pc relative addressing 
    std::vector<int> dispFixupIndexes;
    std::map<std::string, std::vector<int>> freftab;
    int index;
    std::vector<Line*> lines;
    std::vector<LitPoolEntry*> literalPool;
    std::unordered_map<std::string, int> literalPoolIndex;     
    std::string name;
    int offset;
    std::vector<RelocEntry*> relocations;
    int startAddress;

    static int counter;
};

#endif