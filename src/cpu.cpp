#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "../inc/cpu.hpp"
#include "../inc/terminal.hpp"
#include "../inc/timer.hpp"

CPU::CPU() 
    : registers{}, csrs{}
    , terminal(new Terminal())
    , timer(new Timer()) 
    , terminalInterrupt(false)
    , timerInterrupt(false) {
    pc = 0x40000000;
    status = 0;
}

CPU::~CPU() {
    delete terminal;
    delete timer;
}

void CPU::execute() {
    uint8_t b1 = mem[pc];
    uint8_t b2 = mem[pc + 1];
    uint8_t b3 = mem[pc + 2];
    uint8_t b4 = mem[pc + 3];
    pc += 4;

    uint8_t oc = (b1 >> 4) & 0xf;
    uint8_t mode = b1 & 0xf;
    uint8_t a = (b2 >> 4) & 0xf;
    uint8_t b = b2 & 0xf;
    uint8_t c = (b3 >> 4) & 0xf;
    int disp = ((b3 & 0xf) << 8) | b4;
    if (disp & 0x800) disp |= ~0xfff;   // sign-extend the 12-bit field

    // process instruction
    switch (oc) {
        case 0:
            isHalted = true;
            printState();
            return;
        case 1:
            enterInterrupt(4);
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
            // invalid op code, generate interrupt
            enterInterrupt(1);
            break;
    }

    // check for interrupts, handle if needed
    if (!(status & I)) {
        timerInterrupt |= (timer->poll() && !(status & Tr));
        terminalInterrupt |= (terminal->poll() && !(status & Tl));
    }

    if (timerInterrupt) {
        timerInterrupt = false;
        enterInterrupt(2);
    }
    else if (terminalInterrupt) {
        terminalInterrupt = false;
        enterInterrupt(3);
    }
}

bool CPU::halted() {
    return isHalted;
}

void CPU::enterInterrupt(int causeCode) {
    cause = causeCode;
    sp -= 4;
    writeWord(sp, pc);              // push pc
    sp -= 4;
    writeWord(sp, status);          // push status
    status |= I;        // mask interrupts
    pc = handler;
}

// reassembles a word from 4 little-endian bytes
// (lowest address holds the least significant byte)
uint32_t CPU::readWord(uint32_t address) {
    if (address == 0xffffff04) {
        return terminal->readIn();
    }
    uint8_t b0 = mem[address];
    uint8_t b1 = mem[address + 1];
    uint8_t b2 = mem[address + 2];
    uint8_t b3 = mem[address + 3];
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

void CPU::writeByte(uint32_t address, uint8_t byte) {
    mem[address] = byte; 
}

// writes a word to memory in little-endian format
// (lowest address holds the least significant byte)
void CPU::writeWord(uint32_t address, uint32_t word) {
    if (address == 0xffffff00) {
        // term out
        terminal->writeOut(word);
    }
    else if (address == 0xffffff10) {
        //  tim_cfg
        timer->configureTimer(word);
    }
    else {
        mem[address]     = word & 0xff;
        mem[address + 1] = (word >> 8) & 0xff;
        mem[address + 2] = (word >> 16) & 0xff;
        mem[address + 3] = (word >> 24) & 0xff;
    }
}

void CPU::handleCall(uint8_t mode, uint8_t a, uint8_t b, uint8_t c, int disp) {
    switch (mode) {
        case 0:
            // push pc
            sp -= 4;
            writeWord(sp, pc);
            // pc <= gpr[A] + gpr[B] + D
            pc = registers[a] + registers[b] + disp;
            break;
        case 1:
            // push pc
            sp -= 4;
            writeWord(sp, pc);
            // pc <= mem[gpr[A] + gpr[B] + D]
            pc = readWord(registers[a] + registers[b] + disp);
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
            pc = readWord(registers[a] + disp);
            break;
        case 9:
            if (registers[b] == registers[c])
                pc = readWord(registers[a] + disp);
            break;
        case 10:
            if (registers[b] != registers[c])
                pc = readWord(registers[a] + disp);
            break;
        case 11:
            if (registers[b] > registers[c])
                pc = readWord(registers[a] + disp);
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
            writeWord(registers[a] + registers[b] + disp, registers[c]);
            break;
        case 1:
            registers[a] = registers[a] + disp;
            writeWord(registers[a], registers[c]);
            break;
        case 2:
            writeWord(readWord(registers[a] + registers[b] + disp), registers[c]);
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
            registers[a] = readWord(registers[b] + registers[c] + disp);
            break;
        case 3:
            registers[a] = readWord(registers[b]);
            registers[b] = registers[b] + disp;
            break;
        case 4:
            csrs[a] = registers[b];
            break;
        case 5:
            csrs[a] = csrs[b] | disp;
            break;
        case 6:
            csrs[a] = readWord(registers[b] + registers[c] + disp);
            break;
        case 7:
            csrs[a] = readWord(registers[b]);
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
