/*
*   Class used to represent CPU state. Used by the emulator.
*/

#ifndef _CPU_H_
#define _CPU_H_

#include <cstdint>

class CPU {
public:
    void execute(uint32_t* instr);
private:

    // gprs
    uint32_t registers[16];
    uint32_t& pc = registers[15];
    uint32_t& sp = registers[14];

    // csrs
    uint32_t status;        // processor status word
    uint32_t handler;       // interrupt routine address
    uint32_t cause;         // interrupt cause
};
#endif