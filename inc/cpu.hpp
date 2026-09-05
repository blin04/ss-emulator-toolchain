/*
*   Class used to represent CPU state. Used by the emulator.
*/

#ifndef _CPU_H_
#define _CPU_H_

#include <cstdint>
#include <map>

class CPU {
public:
    CPU();

    void execute();
    bool halted();
    void loadWord(uint32_t address, uint32_t word);
private:

    // gprs
    uint32_t    registers[16];
    uint32_t&   pc = registers[15];
    uint32_t&   sp = registers[14];

    // csrs
    uint32_t    csrs[3];
    uint32_t&   status = csrs[0];        // processor status word
    uint32_t&   handler = csrs[1];       // interrupt routine address
    uint32_t&   cause = csrs[2];         // interrupt cause

    // other
    bool        isHalted;

    // memory -- think about this one
    std::map<uint32_t, uint8_t> mem;

    void handleCall(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleJump(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleAtomicSwap(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleArithmetic(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleBitwise(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleShifts(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleStore(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void handleLoad(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp);
    void printState();

};
#endif