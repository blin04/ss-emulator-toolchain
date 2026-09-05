#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "../inc/cpu.hpp"

CPU::CPU() 
    : registers{}, csrs{} {
    pc = 0x40000000;
}

void CPU::execute() {
    uint8_t b1 = mem[pc];
    uint8_t b2 = mem[pc + 1];
    uint8_t b3 = mem[pc + 2];
    uint8_t b4 = mem[pc + 3];
    pc += 4;

    uint8_t oc = b1 & 0xf0;
    uint8_t mode = b1 & 0xf;
    uint8_t a = b2 & 0xf0;
    uint8_t b = b2 & 0xf;
    uint8_t c = b3 & 0xf0;
    int disp = ((b3 & 0xf) << 8) | b4;

    // process instruction
    switch (oc) {
        case 0:
            isHalted = true;
            printState();
            return;
        case 1:
            // interupt
            break;
        case 2:
            handleCall(mode, a, b, c, disp);
            break;
        case 3:
            handleJump(mode, a, b, c, disp);
            break;
        case 4:
            handleAtomicSwap(mode, a, b, c, disp);
            break;
        case 5:
            handleArithmetic(mode, a, b, c, disp);
            break;
        case 6:
            handleBitwise(mode, a, b, c, disp);
            break;
        case 7:
            handleShifts(mode, a, b, c, disp);
            break;
        case 8:
            handleStore(mode, a, b, c, disp);
            break;
        case 9:
            handleLoad(mode, a, b, c, disp);
            break;
        default:
            // note: silent failure
            break;
    }
}

bool CPU::halted() {
    return isHalted;
}

void CPU::loadWord(uint32_t address, uint32_t word) {
    mem[address] = word;
}

void CPU::handleCall(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            // push pc
            mem[sp] = pc; 
            sp -= 4;
            // pc <= gpr[A] + gpr[B] + D
            pc = registers[a] + registers[b] + disp;
            break;
        case 1:
            // push pc
            mem[sp] = pc; 
            sp -= 4;
            // pc <= gpr[A] + gpr[B] + D
            pc = mem[registers[a] + registers[b] + disp];
            break;
        default:
            break;
    }
}

void CPU::handleJump(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            pc = registers[a] + disp;
            break;
        case 1:
            if (registers[b] == registers[c])
                pc = registers[a] + disp;
            break;
        case 2:
            if (registers[b] != registers[c])
                pc = registers[a] + disp;
            break;
        case 3:
            if (registers[b] > registers[c])
                pc = registers[a] + disp;
            break;
        case 8:
            pc = mem[registers[a] + disp];
            break;
        case 9:
            if (registers[b] == registers[c])
                pc = mem[registers[a] + disp];
            break;
        case 10:
            if (registers[b] != registers[c])
                pc = mem[registers[a] + disp];
            break;
        case 11:
            if (registers[b] > registers[c])
                pc = mem[registers[a] + disp];
            break;
    }
} 

void CPU::handleAtomicSwap(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    uint32_t temp = registers[b];
    registers[b] = registers[c];
    registers[c] = temp;
}

void CPU::handleArithmetic(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            registers[a] = registers[b] + registers[c];
            break;    
        case 1:
            registers[a] = registers[b] - registers[c];
            break;    
        case 2:
            registers[a] = registers[b] * registers[c];
            break;    
        case 3:
            registers[a] = registers[b] / registers[c];
            break;    
    }
}

void CPU::handleBitwise(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            registers[a] = ~registers[b];
            break;    
        case 1:
            registers[a] = registers[b] & registers[c];
            break;    
        case 2:
            registers[a] = registers[b] | registers[c];
            break;    
        case 3:
            registers[a] = registers[b] ^ registers[c];
            break;    
    }
}

void CPU::handleShifts(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            registers[a] = registers[b] << registers[c];
            break;
        case 1:
            registers[a] = registers[b] >> registers[c];
            break;
    }
}

void CPU::handleStore(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            mem[registers[a] + registers[b] + disp] = registers[c];
            break;
        case 1:
            mem[mem[registers[a] + registers[b] + disp]] = registers[c];
            break;
        case 2:
            registers[a] = registers[a] + disp;
            mem[registers[a]] = registers[c];
            break;
    }
}

void CPU::handleLoad(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            registers[a] = csrs[b];
            break;
        case 1:
            registers[a] = registers[b] + disp;
            break;
        case 2:
            registers[a] = mem[registers[b] + registers[c] + disp];
            break;
        case 3:
            registers[a] = mem[registers[b]];
            registers[b] = registers[b] = disp;
            break;
        case 4:
            csrs[a] = registers[b];
            break;
        case 5:
            csrs[a] = csrs[b] | disp;
            break;
        case 6:
            csrs[a] = mem[registers[b] + registers[c] + disp];
            break;
        case 7:
            registers[a] = mem[registers[b]];
            registers[b] = registers[b] + disp;
            break;
    }
}

void CPU::printState() {
    std::cout << "------------------------------------------------------------------\n";
    std::cout << "Emulated processor executed halt instruction\n";
    std::cout << "Emulated processor state:\n";

    const int cols = 4, rows = 16 / cols;

    // per-column label width, so '=' lines up within each column
    // even though "r9" and "r10" aren't the same length
    int colWidth[cols] = {0};
    for (int reg = 0; reg < 16; reg++) {
        int c = reg % cols;
        int width = ("r" + std::to_string(reg)).size();
        colWidth[c] = std::max(colWidth[c], width);
    }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int reg = r * cols + c;
            std::cout << std::right << std::setw(colWidth[c]) << ("r" + std::to_string(reg))
                       << "=0x" << std::hex << std::setfill('0') << std::setw(8) << registers[reg]
                       << std::dec << std::setfill(' ');
            if (c != cols - 1) std::cout << "    ";
        }
        std::cout << "\n";
    }
}
