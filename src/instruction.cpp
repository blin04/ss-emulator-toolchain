#include "../inc/instruction.hpp"

Instruction::Instruction(uint8_t oc, uint8_t mod, uint8_t regA, uint8_t regB, uint8_t regC, int disp) {
    b1 = (oc << 4) | mod;
    b2 = (regA << 4) | regB;
    b3 = (regC << 4) | ((disp >> 8) & 0xf);
    b4 = disp & 0xff;
}

std::vector<uint8_t> Instruction::generateBytes() { return std::vector<uint8_t>({b1, b2, b3, b4}); }