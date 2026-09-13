/*
*   Class used to represent CPU state. Used by the emulator.
*/

#ifndef _CPU_H_
#define _CPU_H_

#include <cstdint>
#include <map>

class Terminal;
class Timer;

class CPU {
public:
    CPU();
    ~CPU();

    void execute();
    bool halted();
    uint32_t readWord(uint32_t address);
    void writeByte(uint32_t address, uint8_t byte);
    void writeWord(uint32_t address, uint32_t word);
private:

    // timer & terminal
    Timer*      timer;
    Terminal*   terminal;

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
    bool        timerInterrupt;
    bool        terminalInterrupt;

    // constants
    const int   I = 0b100;
    const int   Tl = 0b10;
    const int   Tr = 1;

    // memory, byte-addressable
    std::map<uint32_t, uint8_t> mem;

    void enterInterrupt(int cause);
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