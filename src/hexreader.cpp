#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../inc/hexreader.hpp"

void HexReader::load(const std::string& path, CPU& cpu) {
    std::ifstream source(path);
    if (!source.is_open())
        throw std::runtime_error("could not open hex file: " + path);

    std::string line;
    while (std::getline(source, line)) {
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        uint32_t address = std::stoul(line.substr(0, colon), nullptr, 16);

        std::istringstream bytes(line.substr(colon + 1));
        std::vector<uint8_t> byteValues;
        std::string byteToken;
        while (bytes >> byteToken)
            byteValues.push_back(std::stoi(byteToken, nullptr, 16));

        for (size_t i = 0; i + 4 <= byteValues.size(); i += 4) {
            uint32_t word = (byteValues[i]     << 24) | (byteValues[i + 1] << 16)
                           | (byteValues[i + 2] << 8)  |  byteValues[i + 3];
            cpu.loadWord(address + i, word);
        }
    }
}
