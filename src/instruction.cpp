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

void Instruction::haltHandler() {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0, 0, 0, 0, 0, 0)
    );
}

void Instruction::intHandler() {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(1, 0, 0, 0, 0, 0)
    );
}

void Instruction::iretHandler() {
    // pop pc
    uint8_t inst = 0b1001;          // data loading instr
    uint8_t mode = 0b0011;
    uint8_t a = Instruction::GPR::PC;
    uint8_t b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 0)
    );

    // pop status
    mode = 0b0111;
    a = Instruction::CSR::status;
    b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(inst, mode, a, b, 0, 0)
    );
}

void Instruction::retHandler() {
    // pop pc
    uint8_t inst = 0b1001;          // data loading instr
    uint8_t mode = 0b0011;
    uint8_t a = Instruction::GPR::PC;
    uint8_t b = Instruction::GPR::SP;
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(
            0b1001, 
            0b0011, 
            Instruction::GPR::PC, 
            Instruction::GPR::SP, 
            0, 0)
    );
}

void Instruction::notHandler(int op) {
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b0110, 0, op, op, 0, 0)
    );
}

void Instruction::pushHandler(int op) {
    // sp <= sp - 4, mem32[sp] <= gpr
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1000, 1, Instruction::GPR::SP, 0, op, -4)
    );
}

void Instruction::popHandler(int op) {
    // gpr <= mem32[sp], sp <= sp + 4
    ObjectFile::getCurrentSection()->addLine(
        new Instruction(0b1001, 3, op, Instruction::GPR::SP, 0, 4)
    );
}

void Instruction::callHandler(int op) {

}

void Instruction::jmpHandler(int op) {

}