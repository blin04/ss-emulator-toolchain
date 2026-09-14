#include "../inc/instruction.hpp"
#include "../inc/objfile.hpp"
#include "../inc/section.hpp"

Instruction::Instruction(uint8_t oc, uint8_t mod, uint8_t regA, uint8_t regB, uint8_t regC, int disp) {
    b1 = (oc << 4) | mod;
    b2 = (regA << 4) | regB;
    b3 = (regC << 4) | ((disp >> 8) & 0xf);
    b4 = disp & 0xff;
}

std::vector<uint8_t> Instruction::generateBytes() { return std::vector<uint8_t>({b1, b2, b3, b4}); }

int Instruction::haltHandler() {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0, 0, 0, 0, 0, 0)
    );
    return 1;
}

int Instruction::intHandler() {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(1, 0, 0, 0, 0, 0)
    );
    return 1;
}

int Instruction::iretHandler() {
    // pop status
    uint8_t inst = 0b1001;          // data loading instr
    uint8_t mode = 0b0111;
    uint8_t a = Instruction::CSR::status;
    uint8_t b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 4)
    );

    // pop pc
    mode = 0b0011;
    a = Instruction::GPR::PC;
    b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 4)
    );

    return 2;
}

int Instruction::retHandler() {
    // pop pc
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(
            0b1001,
            0b0011,
            Instruction::GPR::PC,
            Instruction::GPR::SP,
            0, 4)
    );
    return 1;
}

int Instruction::notHandler(int op) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0110, 0, op, op, 0, 0)
    );
    return 1;
}

int Instruction::pushHandler(int op) {
    // sp <= sp - 4, mem32[sp] <= gpr
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1000, 1, Instruction::GPR::SP, 0, op, -4)
    );
    return 1;
}

int Instruction::popHandler(int op) {
    // gpr <= mem32[sp], sp <= sp + 4
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1001, 3, op, Instruction::GPR::SP, 0, 4)
    );
    return 1;
}

int Instruction::callHandler(int op, bool fromPool) {
    // push pc, pc <= op

    // if operand is referenced from literal
    // pool PC relative addressing is used
    uint8_t mode = fromPool ? 1 : 0;
    uint8_t regA = fromPool ? 15 : 0;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0010, mode, regA, 0, 0, op),
        fromPool
    );
    return 1;
}

int Instruction::jmpHandler(int op, bool fromPool) {
    // pc <= op
    uint8_t mode = fromPool ? 8 : 0;
    uint8_t regA = fromPool ? 15 : 0;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0011, mode, regA, 0, 0, op),
        fromPool
    );
    return 1;
}

int Instruction::beqHandler(int gpr1, int gpr2, int op, bool fromPool) {
    // if (gpr1 == gpr2) pc <= operand
    uint8_t mode = fromPool ? 9 : 1;
    uint8_t regA = fromPool ? 15 : 0;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0011, mode, regA, gpr1, gpr2, op),
        fromPool
    );
    return 1;
}

int Instruction::bneHandler(int gpr1, int gpr2, int op, bool fromPool) {
    // if (gpr1 != gpr2) pc <= operand
    uint8_t mode = fromPool ? 10 : 2;
    uint8_t regA = fromPool ? 15 : 0;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0011, mode, regA, gpr1, gpr2, op),
        fromPool
    );
    return 1;
}

int Instruction::bgtHandler(int gpr1, int gpr2, int op, bool fromPool) {
    // if (gpr1 > gpr2) pc <= operand
    uint8_t mode = fromPool ? 11 : 3;
    uint8_t regA = fromPool ? 15 : 0;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0011, mode, regA, gpr1, gpr2, op),
        fromPool
    );
    return 1;
}

int Instruction::xchngHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0100, 0, 0, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::addHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0101, 0, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::subHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0101, 1, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::mulHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0101, 2, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::divHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0101, 3, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::andHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0110, 1, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::orHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0110, 2, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::xorHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0110, 3, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::shlHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0111, 0, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::shrHandler(int gprS, int gprD) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0111, 1, gprD, gprD, gprS, 0)
    );
    return 1;
}

int Instruction::csrrdHandler(int csr, int gpr) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1001, 0, gpr, csr, 0, 0)
    );
    return 1;
}

int Instruction::csrwrHandler(int gpr, int csr) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1001, 0b0100, csr, gpr, 0, 0)
    );
    return 1;
}

int Instruction::ldHandler(bool fromMemory, int gprBase, int disp, int gprDest, bool fromPool) {
    uint8_t mode = (fromMemory || fromPool) ? 0b0010 : 0b0001;
    uint8_t regBase = fromPool ? 15 : gprBase;

    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1001, mode, gprDest, regBase, 0, disp),
        fromPool
    );

    // symbol / literal value
    // dereferencing needed
    if (fromMemory && fromPool) {
        ObjectFile::getCurrentSection()->addLine(
            new Instruction(0b1001, 0b0010, gprDest, gprDest, 0, 0),
            false
        );
        return 2;
    }

    return 1;
}

// ST makes no sense with $literal
int Instruction::stHandler(bool fromMemory, int gprBase, int disp, int gprSource, bool fromPool) {
    uint8_t regBase = fromPool ? 15 : gprBase;


    if (fromMemory && fromPool) {
        // litreal, simbol
        ObjectFile::getCurrentSection()->addLine(
            new Instruction(0b1000, 0b0010, regBase, 0, gprSource, disp),
            fromPool
        );
        return 1;
    }
    else {
        if (fromMemory) {
            // literal / simbol     (non-pool)
            // [%reg]
            // [%reg + simbol / literal]
            ObjectFile::getCurrentSection()->addLine(
                new Instruction(0b1000, 0, regBase, 0, gprSource, disp),
                fromPool
            );
            return 1;
        }
        else {
            // note: should raise error for $literal and
            // update symbol value for $simbol
            // for now it just fails silently
            if (gprBase != 0) {
                // %reg
                ObjectFile::getCurrentSection()->addLine(
                    new Instruction(0b1001, 1, gprBase, gprSource, 0, 0),
                    fromPool
                );
                return 1;
            }
        }
    }

    // nothing emitted (e.g. silently-ignored st with $literal)
    return 0;
}