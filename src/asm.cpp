#include <iostream>
#include <cstdio>

#include "../inc/objfile.hpp"

extern FILE* yyin;
extern void yyparse();

int main() {
    FILE* file = fopen("./test/test.s", "r");
    if (!file) {
        std::cout << "error: failed opening source file\n";
        return 1;
    }

    ObjectFile* output = ObjectFile::getInstance();

    yyin = file;
    yyparse();

    fclose(file);

    // Generate object file
    output->generate();

    return 0;
}