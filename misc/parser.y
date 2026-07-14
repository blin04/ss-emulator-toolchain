%code requires {
  #include "../inc/interface.h"
}

%{
  #include <stdio.h>
  #include <string.h>
  
  #include "../inc/interface.h"

  int yylex();
  void yyerror(const char *);

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

  int line_num = 1;
  extern int location_counter;  // address inside a section
  int total_offset = 0;         // total offset from the beginning of the file

%}

%union {
  int ival;
  char* sval;
  char** arrval;      // null-terminated array of pointers to symbols
  Operand oprval;
}

/* declaring used tokens */

%token <ival> HALT <ival> INT <ival> IRET <ival> CALL 
%token <ival> RET <ival> JMP <ival> BEQ <ival> BNE 
%token <ival> BGT <ival> PUSH <ival> POP <ival> XCHNG 
%token <ival> ADD <ival> SUB <ival> MUL <ival> DIV 
%token <ival> NOT <ival> AND <ival> OR <ival> XOR 
%token <ival> SHL <ival> SHR <ival> LD <ival> ST 
%token <ival> CSRRD <ival> CSRWR
%token COMMA COMMENT <sval> COLON DOLLAR <ival> LITERAL
%token LPAR MINUS NL PLUS RPAR <sval> SYMBOL <sval> STRING
%token ASCII END EQU EXTERN GLOBAL SECTION SKIP WORD
%token CAUSE HANDLER PC <ival> REG SP STATUS 

%left PLUS MINUS

%type <ival> directive      // value to add to location counter
%type <ival> exp;
%type <sval> label;
%type <arrval> symbol_list
%type <arrval> symbol_or_literal_list
%type <ival> zero_op_stmt;
%type <ival> data_one_op_stmt;
%type <ival> jmp_one_op_stmt;
%type <ival> two_op_stmt;
%type <ival> three_op_stmt
%type <ival> gpr;
%type <ival> csr;
%type <oprval> jump_operand;
%type <oprval> data_operand

%%

/* grammar rules */

program: 
    line { line_num++; }
  | program NL line { line_num++; }
  ;

line:
    /* empty */
  | directive comment { location_counter += $1; }
  | statement comment { location_counter += 4; }
  | label comment { 
      if (externSymbol($1)) YYERROR;
      defineSymbol($1); 
    }
  | label directive comment { 
      if (externSymbol($1)) YYERROR;
      defineSymbol($1); 
      location_counter += $2; 
    }
  | label statement comment { 
      if (externSymbol($1)) YYERROR;
      defineSymbol($1); 
      location_counter += 4; 
    }
  | COMMENT
  ;

comment:
    /* empty */
  | COMMENT
  
label:
  SYMBOL COLON { $$ = $1; }
  ;
  
// semantic value of <directive> non-terminal
// is the number of bytes it generates 
directive:
    ASCII STRING { addAsciiDirective($2); $$ = strlen($2) - 2; free($2); }    // -2 because of " and "
  | END { YYACCEPT; /* end parsing successfully */ }
  | EQU SYMBOL COMMA exp { defineSymbol($2, true); $$ = 0;}
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
    zero_op_stmt { zeroOpStatementHandler($1); }
  | one_op_stmt
  | two_op_stmt gpr COMMA gpr { twoOpStatementHandler($1, $2, $4); }
  | three_op_stmt gpr COMMA gpr COMMA jump_operand { threeOpStatementHandler($1, $2, $4, $6); }
  | LD data_operand COMMA gpr { memoryStatementHandler($1, $2, $4); }
  | ST gpr COMMA data_operand { memoryStatementHandler($1, $4, $2); }
  | CSRRD csr COMMA gpr { twoOpStatementHandler($1, $2, $4); }
  | CSRWR gpr COMMA csr { twoOpStatementHandler($1, $2, $4); }
  ;

zero_op_stmt: 
    HALT
  | INT
  | IRET
  | RET
  ;                  

one_op_stmt: 
    data_one_op_stmt gpr { oneOpStatementHandler($1, $2); }
  | jmp_one_op_stmt jump_operand { oneOpJumpStatementHandler($1, $2); }
  ;

two_op_stmt:
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

three_op_stmt: 
    BEQ 
  | BNE 
  | BGT
  ;

data_one_op_stmt:
    PUSH
  | POP
  | NOT
  ;

jmp_one_op_stmt:
    CALL
  | JMP
  ;

jump_operand:
    LITERAL {
      $$.fromMemory = false;
      $$.gpr = 0;
      $$.disp = $1; 
      $$.symbol = NULL;
    }
  | SYMBOL { 
      $$.fromMemory = false;
      $$.gpr = 0;
      $$.disp = getSymbolValue($1); 
      // $$.absolute = isAbsolute($1);
      $$.symbol = NULL;
    }
  ;

data_operand:
    DOLLAR LITERAL { 
      $$.fromMemory = false; 
      $$.gpr = 0; 
      $$.disp = $2; 
      // $$.absolute = true;
      $$.symbol = NULL;
    }
  | DOLLAR SYMBOL { 
      $$.fromMemory = false, 
      $$.gpr = 0; 
      $$.disp = getSymbolValue($2); 
      // $$.absolute = isAbsolute($2);
      $$.symbol = strdup($2);
    }
  | LITERAL { 
      $$.fromMemory = true; 
      $$.gpr = 0; 
      $$.disp = $1; 
      // $$.absolute = true;
      $$.symbol = NULL;
    }
  | SYMBOL { 
      $$.fromMemory = true; 
      $$.gpr = 0; 
      $$.disp = getSymbolValue($1); 
      // $$.absolute = isAbsolute($1);
      $$.symbol = strdup($1);
    }
  | gpr { 
      $$.fromMemory = false; 
      $$.gpr = $1; 
      $$.disp = 0; 
      // $$.defined = true;
      $$.symbol = NULL;
    }
  | LPAR gpr RPAR { 
      $$.fromMemory = true; 
      $$.gpr = $2; 
      $$.disp = 0; 
      // $$.defined = true;
      $$.symbol = NULL;
    }
  | LPAR gpr PLUS LITERAL RPAR { 
      // 12b signed values are [-2^11, 2^11 - 1]
      if ($4 >= (1 << 11) || $4 < -(1 << 11)) {
        yyerror("literal value too large: displacement for base register addressing must fit as a signed value in 12b");
        YYERROR;
      }

      $$.fromMemory = true; 
      $$.gpr = $2; 
      $$.disp = $4; 
      // $$.defined = true;
      $$.symbol = NULL;
    }
  | LPAR gpr PLUS SYMBOL RPAR { 
      if (isDefined($4) && 
        (getSymbolValue($4) >= (1 << 11) || 
        getSymbolValue($4) < -(1 << 11))) {

        yyerror("symbol value too large: displacement for base register addressing must fit as a signed value in 12b");
        YYERROR;
      }

      $$.fromMemory = true; 
      $$.gpr = $2; 
      $$.disp = getSymbolValue($4); 
      // $$.absolute = isAbsolute($4);
      $$.symbol = strdup($4);
    }
  ;

exp:
    exp PLUS exp {$$ = $1 + $3; }
  | exp MINUS exp { $$ = $1 - $3; }
  | SYMBOL { $$ = getSymbolValue($1); }
  | LITERAL { $$ = $1; }
  ;

// for csr and gpr index of the used register
// (per CPU architecture) is returned

gpr:
    REG
  | SP { $$ = 14; }
  | PC { $$ = 15; }
  ;

csr:    
    HANDLER { $$ = 1; }   
  | STATUS { $$ = 0; }
  | CAUSE { $$ = 2; }
  ;

%%

void yyerror(const char* c) {
  printf("Parse error: %s\n", c);

  // causes seg fault for some reason: 
  // printf("(token value %s, at line number %d) \n", yylval, line_num);
}