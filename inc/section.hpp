#ifndef _SECTION_H_
#define _SECTION_H_

#include <cstdint>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

enum RelocType { ABS, REL };

class Line;

class Section {
public:
    Section(std::string name, int offset);
    ~Section();

    // returns index of the added relocation
    int addRelocation(int offset, RelocType type, int symbolIndex, int addend);
    void addLine(Line*, bool fromPool = false);
    // returns index of the added value in the pool
    int addLiteralPoolValue(int value, const char* symbol);
    void addForwardReference(std::string symbol, int location);
    void backpatch();
    int getSectionID();
    void serialize(std::ofstream& file);
private:

    typedef struct {
        int         offset;         // from section start in bytes
        RelocType   type;
        int         symbol;         // index in SymbolTable
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

    // section contents in bytes
    std::vector<uint8_t> section_bytes;

    // list of **indexes** of instructions that access 
    // symbols in literal pool with pc relative addressing 
    std::vector<int> dispFixupIndexes;
    // table for symbols that are forward referenced
    // for each symbol byte positions where it's
    // value should be put are stored
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