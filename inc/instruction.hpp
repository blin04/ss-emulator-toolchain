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

    // todo: add functions for corrections 
    // depending on displacement values

    // Functions for instruction creation

    static void haltHandler();
    static void intHandler();
    static void iretHandler();
    static void retHandler();

    static void notHandler(int op);
    static void pushHandler(int op);
    static void popHandler(int op);

    static void callHandler(int op, bool fromPool);
    static void jmpHandler(int op, bool fromPool);

    static void beqHandler(int gpr1, int gpr2, int op, bool fromPool);
    static void bneHandler(int gpr1, int gpr2, int op, bool fromPool);
    static void bgtHandler(int gpr1, int gpr2, int op, bool fromPool);

    static void xchngHandler(int gpr1, int gpr2);
    static void addHandler(int gpr1, int gpr2);
    static void subHandler(int gpr1, int gpr2);
    static void mulHandler(int gpr1, int gpr2);
    static void divHandler(int gpr1, int gpr2);
    static void andHandler(int gpr1, int gpr2);
    static void orHandler(int gpr1, int gpr2);
    static void xorHandler(int gpr1, int gpr2);
    static void shlHandler(int gpr1, int gpr2);
    static void shrHandler(int gpr1, int gpr2);

    static void ldHandler(bool fromMemory, int gprBase, int disp, int gprSource, bool fromPool);
    static void stHandler(bool fromMemory, int gprBase, int disp, int gprSource, bool fromPool);

    static void csrrdHandler(int csr, int gpr);
    static void csrwrHandler(int gpr, int csr);

private:
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint8_t b4;
};

#endif