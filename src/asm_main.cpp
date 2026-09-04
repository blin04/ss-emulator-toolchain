#include <iostream>
#include <cstdio>

#include "../inc/objfile.hpp"

extern FILE* yyin;
extern void yyparse();

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    std::string filename = argv[1];
    std::string path = "./test/" + filename;
    FILE* file = fopen(path.c_str(), "r");
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