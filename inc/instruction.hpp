#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include "line.hpp"

class Instruction : public Line {
public: 
    Instruction(uint8_t oc, uint8_t mod, uint8_t regA, uint8_t regB, uint8_t regC, int disp);
    std::vector<uint8_t> generateBytes();

    // todo: should be made global so that
    // the parser can also access them 
    // and pass them through the interface
    enum GPR : uint8_t {
        R0 = 0,
        R1,
        R2,
        R3,
        R4,
        R5,
        R6,
        R7,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
        SP = R14,
        PC = R15
    };

    enum CSR : uint8_t {
        status = 0,
        handler,
        cause
    };



    // instruction handlers return the number of machine
    // instructions they emit
    static int haltHandler();
    static int intHandler();
    static int iretHandler();
    static int retHandler();

    static int notHandler(int op);
    static int pushHandler(int op);
    static int popHandler(int op);

    static int callHandler(int op, bool fromPool);
    static int jmpHandler(int op, bool fromPool);

    static int beqHandler(int gpr1, int gpr2, int op, bool fromPool);
    static int bneHandler(int gpr1, int gpr2, int op, bool fromPool);
    static int bgtHandler(int gpr1, int gpr2, int op, bool fromPool);

    static int xchngHandler(int gpr1, int gpr2);
    static int addHandler(int gpr1, int gpr2);
    static int subHandler(int gpr1, int gpr2);
    static int mulHandler(int gpr1, int gpr2);
    static int divHandler(int gpr1, int gpr2);
    static int andHandler(int gpr1, int gpr2);
    static int orHandler(int gpr1, int gpr2);
    static int xorHandler(int gpr1, int gpr2);
    static int shlHandler(int gpr1, int gpr2);
    static int shrHandler(int gpr1, int gpr2);

    static int ldHandler(bool fromMemory, int gprBase, int disp, int gprSource, bool fromPool);
    static int stHandler(bool fromMemory, int gprBase, int disp, int gprSource, bool fromPool);

    static int csrrdHandler(int csr, int gpr);
    static int csrwrHandler(int gpr, int csr);

private:
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;
};

#endif