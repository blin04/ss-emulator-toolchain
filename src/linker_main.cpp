#include <iostream>
#include <string>

#include "../inc/linker.hpp"

int main(int argc, char** argv) {
    std::string outputPath = "out_link.txt";
    Linker* linker = Linker::getInstance();

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-relocatable") {
            linker->makeRelocatable();
        }
        else if (arg == "-o") {
            // todo: validate i + 1 < argc

            if (i + 1 < argc)
                outputPath = argv[++i];
            else {
                std::cout << "error: no output file given" << "\n";
                return 0;
            }
        }
        else if (arg.find("-place=", 0) == 0) {
            // place=<section>@<address>
            int at = arg.find('@');
            // 7 is the index where section name starts
            std::string section = arg.substr(7, at - 7);
            std::string address_str = arg.substr(at + 1, arg.size());
            uint32_t address;
            try {
                address = std::stoi(address_str);
            }
            catch (std::invalid_argument e) {
                std::cout << "error: address for section " << section << "can't be converted to numeric format\n";
                return 1; 
            }
            linker->setPlacement(section, address);
        }
        else if (arg == "-hex") {
            linker->makeHex();
        }
        else {
            linker->addInputFile(arg);
        }
    }

    linker->link();
    return 0;
}
