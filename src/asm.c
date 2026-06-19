#include <stdio.h>

extern int yyparse();
extern FILE* yyin;

int main() {
    FILE* file = fopen("./test/main.s", "r");

    if (!file) {
        printf("error: failed opening source file\n");
        return 1;
    }

    yyin = file;
    yyparse();

    fclose(file);

    return 0;
}