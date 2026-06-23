%{
  #include <stdio.h>
  #include <string.h>
  
  #include "../inc/interface.h"

  int yylex();
  void yyerror(const char *);

  int line_num = 1;
  int location_counter = 0;     // address inside a section
  int total_offset = 0;         // total offset from the beginning of the file

  // helper function used for parsing
  // of .extern directive
  // checks if there is no already defined 
  // symbol in the given array, since a defined
  // symbol can't be declared extern 
  int noDefinedSymbol(char** symbs) {
    for (int i = 0; symbs[i] != NULL; i++) {
      if (isDefined(symbs[i])) {
        // todo: add symbol name into error message
        yyerror("defined symbol can't be declared as extern");
        return 0;
      }
    } 
    return 1;
  }

  // analogous to above function, this one
  // checks if the given symbol is declared as external
  // due to implementation details it only takes
  // one symbol as an argument, not the whole array
  int externSymbol(char* symb) {
    if (isExtern(symb)) {
      // todo: add symbol name into error message
      yyerror("definition of a symbol previously declared as extern");
      return 1;
    }
    return 0;
  }

%}

%union {
  int ival;
  long lval;
  char* sval;
  char** arrval;      // null-terminated array of pointers to symbols
}

/* declaring used tokens */

%token HALT INT IRET CALL RET JMP BEQ BNE 
%token BGT PUSH POP XCHNG ADD SUB MUL DIV 
%token NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token COMMA COMMENT <sval> COLON DOLLAR <lval> LITERAL
%token LPAR MINUS NL PLUS RPAR <sval> SYMBOL <sval> STRING
%token ASCII END EQU EXTERN GLOBAL SECTION SKIP WORD
%token CAUSE HANDLER PC REG SP STATUS 

%left PLUS MINUS

%type <ival> directive      // value to add to location counter
%type <ival> exp;
%type <sval> label;
%type <arrval> symbol_list
%type <arrval> symbol_or_literal_list

%%

/* grammar rules */

program: 
    line { line_num++; }
  | program NL line { line_num++; }
  ;

line:
    /* empty */
  | directive comment { location_counter += $1; }
  | statement comment { addInstruction(); location_counter += 4; }
  | label comment { defineSymbol($1, location_counter); }
  | label directive comment { 
      if (externSymbol($1)) YYERROR;
      defineSymbol($1, location_counter); 
      location_counter += $2; 
    }
  | label statement comment { defineSymbol($1, location_counter); addInstruction(); location_counter += 4; }
  | COMMENT
  ;

comment:
    /* empty */
  | COMMENT
  
label:
  SYMBOL COLON { $$ = $1; }
  ;
  
directive:
    ASCII STRING { addAsciiDirective($2); $$ = strlen($2) - 2; free($2); }    // -2 because of " and "
  | END { YYACCEPT; /* end parsing successfully */ }
  | EQU SYMBOL COMMA exp { defineSymbol($2, $4, true); $$ = 0;}
  | EXTERN symbol_list { if (!noDefinedSymbol($2)) { YYERROR; } declareSymbolsExtern($2); $$ = 0; }
  | GLOBAL symbol_list { declareSymbolsGlobal($2); $$ = 0; }
  | SECTION SYMBOL { 
      total_offset += location_counter; 
      startNewSection($2, total_offset); 
      location_counter = 0;       // resets the location counter
      $$ = 0;
    }
  | SKIP LITERAL { addSkipDirective($2); $$ = $2;}
  | WORD symbol_or_literal_list { $$ = addWordDirective($2); }
  ;

symbol_list:
    SYMBOL {
      char** arr = (char**)malloc(2 * sizeof(char*));
      arr[0] = strdup($1);
      arr[1] = NULL;
      $$ = arr;
    }
  | symbol_list COMMA SYMBOL {
      int count = 0;
      while ($1[count] != NULL) count++;
      char** arr = (char**)realloc($1, (count + 2) * sizeof(char*));

      arr[count] = strdup($3);
      arr[count + 1] = NULL;

      $$ = arr;
  }
  ;

symbol_or_literal_list:
    SYMBOL {
      char** arr = (char**)malloc(2 * sizeof(char*));
      arr[0] = strdup($1);
      arr[1] = NULL;
      $$ = arr;
    }
  | LITERAL {
      char** arr = (char**)malloc(2 * sizeof(char*));
      char buf[32];
      sprintf(buf, "%ld", $1);    // maybe not ideal to convert number to string and back to string
      arr[0] = strdup(buf);
      arr[1] = NULL;
      $$ = arr;
    }
  | symbol_or_literal_list COMMA SYMBOL {
      int count = 0;
      while ($1[count] != NULL) count++;
      char** arr = (char**)realloc($1, (count + 2) * sizeof(char*));

      arr[count] = strdup($3);
      arr[count + 1] = NULL;

      $$ = arr;
    }
  | symbol_or_literal_list COMMA LITERAL {
      int count = 0;
      while ($1[count] != NULL) count++;
      char** arr = (char**)realloc($1, (count + 2) * sizeof(char*));

      char buf[32];
      sprintf(buf, "%ld", $3);    // same consideration as above
      arr[count] = strdup(buf);
      arr[count + 1] = NULL;

      $$ = arr;
    }
  ;

statement: 
    zero_op_instr
  | one_op_instr
  | two_op_instr gpr COMMA gpr
  | three_op_instr gpr COMMA gpr COMMA jump_operand
  | LD data_operand COMMA gpr
  | ST gpr COMMA data_operand
  | CSRRD csr COMMA gpr
  | CSRWR gpr COMMA csr
  ;

zero_op_instr: 
    HALT 
  | INT 
  | IRET 
  | RET 
  ;

one_op_instr: 
    data_one_op_instr gpr 
  | jmp_one_op_instr jump_operand
  ;

two_op_instr:
    XCHNG 
  | ADD
  | SUB 
  | MUL 
  | DIV 
  | AND 
  | OR 
  | XOR 
  | SHL 
  | SHR
  ;

three_op_instr: 
    BEQ 
  | BNE 
  | BGT
  ;

data_one_op_instr:
    PUSH
  | POP
  | NOT
  ;

jmp_one_op_instr:
    CALL 
  | JMP
  ;

jump_operand:
    LITERAL
  | SYMBOL
  ;

data_operand:
    DOLLAR LITERAL
  | DOLLAR SYMBOL
  | LITERAL
  | SYMBOL
  | gpr
  | LPAR gpr RPAR
  | LPAR gpr PLUS LITERAL RPAR
  | LPAR gpr PLUS SYMBOL RPAR

exp:
    exp PLUS exp {$$ = $1 + $3; }
  | exp MINUS exp { $$ = $1 - $3; }
  | SYMBOL { $$ = getSymbolValue($1); }
  | LITERAL { $$ = $1; }

gpr:
    REG
  | SP
  | PC
  ;

csr:
    HANDLER
  | STATUS
  | CAUSE
  ;

%%

void yyerror(const char* c) {
  printf("Parse error: %s\n", c);
  printf("(token value %s, at line number %d) \n", yylval, line_num);
}