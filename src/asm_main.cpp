#include <iostream>
#include <cstdio>

#include "../inc/objfile.hpp"

extern FILE* yyin;
extern void yyparse();

// ./asembler [-o <output_file>] input_file
int main(int argc, char** argv) {
    ObjectFile* output = ObjectFile::getInstance();

    std::string filename = argv[1];
    if (filename == "-o") {
        output->setOutput(argv[2]);
        filename = argv[3];
    } 

    // temporarily reading from test directory 
    std::string path = "./test/" + filename;

    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        std::cout << "error: failed opening source file\n";
        return 1;
    }

    yyin = file;
    yyparse();

    fclose(file);

    // Generate object file
    output->generate();

    return 0;
}