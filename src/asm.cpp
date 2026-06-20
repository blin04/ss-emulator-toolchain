#include <iostream>
#include <cstdio>

extern FILE* yyin;
extern void yyparse();

int main() {
    FILE* file = fopen("./test/main.s", "r");
    if (!file) {
        std::cout << "error: failed opening source file\n";
        return 1;
    }

    yyin = file;
    yyparse();

    fclose(file);

    return 0;
}