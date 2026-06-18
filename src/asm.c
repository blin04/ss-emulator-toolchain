#include <stdio.h>

extern int yyparse();
extern FILE* yyin;

int main() {
    yyparse();
}