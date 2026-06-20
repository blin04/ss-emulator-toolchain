#include "../inc/directives.hpp"

SkipDirective::SkipDirective(int bytesCount) : count(bytesCount) {}

std::vector<uint8_t> SkipDirective::generateBytes() {
    std::vector<uint8_t> bytes(count, 0);
    return bytes;
}