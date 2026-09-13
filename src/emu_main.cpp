#include <iostream>

#include "../inc/cpu.hpp"
#include "../inc/hexreader.hpp"
#include "../inc/terminal.hpp"

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cout << "error: too many arguments\n";
        std::cout << "usage: `emulator <hex_file>`\n";
        return 0;
    }

    CPU cpu;

    try {
        HexReader::load(argv[1], cpu);
    }
    catch (const std::runtime_error& e) {
        std::cout << "error: " << e.what() << "\n";
        return 0;
    }

    while (!cpu.halted()) {
        cpu.execute();
        // interrupts
    }
    
    return 0;
}