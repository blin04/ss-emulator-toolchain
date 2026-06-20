#include "../inc/instructions.hpp"

std::vector<uint8_t> HaltInstruction::generateBytes() {
    return {0, 0, 0, 0}; 
}