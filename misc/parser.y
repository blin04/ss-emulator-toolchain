%code requires {
  #include "../inc/interface.h"
}

%{
  #include <stdarg.h>
  #include <stdio.h>
  #include <string.h>

  #include "../inc/interface.h"

  int yylex();
  void yyerror(const char *);

  // formats a message with printf-style arguments and forwards
  // it to yyerror; kept separate from yyerror itself so callers
  // outside of yyparse (e.g. this file's helper functions) can
  // still produce a message without needing YYERROR, which is
  // only valid inside a grammar action
  void yyerrorf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    yyerror(buf);
  }

  // reports a formatted parse error and aborts yyparse;
  // only usable directly inside a grammar action, since
  // YYERROR expands to a jump within yyparse's own body
  #define PARSE_ERROR(...) do { yyerrorf(__VA_ARGS__); YYERROR; } while (0)

  // helper function used for parsing of .extern directive
  // returns the first already-defined symbol found in the
  // given array, or NULL if none are defined, since a defined
  // symbol can't be declared extern
  char* firstDefinedSymbol(char** symbs) {
    for (int i = 0; symbs[i] != NULL; i++) {
      if (isDefined(symbs[i])) return symbs[i];
    }
    return NULL;
  }

  // analogous to above function, this one
  // checks if the given symbol is declared as external
  // due to implementation details it only takes
  // one symbol as an argument, not the whole array
  int externSymbol(char* symb) {
    return isExtern(symb);
  }

  int line_num = 1;
  extern int location_counter;  // address inside a section
  int total_offset = 0;         // total offset from the beginning of the file

%}

// makes Bison-detected syntax errors (unexpected token)
// report which token was unexpected and what was expected,
// instead of just the generic "syntax error"
%define parse.error verbose

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
    line {  line_num++; printf("parsed %d lines\n", line_num); }
  | program NL line { line_num++; printf("parsed line %d \n", line_num);}
  ;

line:
    /* empty */
  | directive comment { location_counter += $1; }
  | statement comment { location_counter += 4; }
  | label comment {
      if (externSymbol($1)) PARSE_ERROR("definition of symbol '%s' previously declared as extern", $1);
      defineSymbol($1, location_counter);
    }
  | label directive comment {
      if (externSymbol($1)) PARSE_ERROR("definition of symbol '%s' previously declared as extern", $1);
      defineSymbol($1, location_counter);
      location_counter += $2;
    }
  | label statement comment {
      if (externSymbol($1)) PARSE_ERROR("definition of symbol '%s' previously declared as extern", $1);
      defineSymbol($1, location_counter);
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
  | EQU SYMBOL COMMA exp { defineSymbol($2, $4, true); $$ = 0;}
  | EXTERN symbol_list {
      char* bad = firstDefinedSymbol($2);
      if (bad) PARSE_ERROR("defined symbol '%s' can't be declared as extern", bad);
      declareSymbolsExtern($2);
      $$ = 0;
    }
  | GLOBAL symbol_list { declareSymbolsGlobal($2); $$ = 0; }
  | SECTION SYMBOL { 
      total_offset += location_counter; 
      startNewSection($2, total_offset); 
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
      $$.symbol = strdup($1);
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
      if ($4 >= (1 << 11) || $4 < -(1 << 11))
        PARSE_ERROR("literal value too large: displacement for base register addressing must fit as a signed value in 12b");

      $$.fromMemory = true;
      $$.gpr = $2;
      $$.disp = $4;
      // $$.defined = true;
      $$.symbol = NULL;
    }
  | LPAR gpr PLUS SYMBOL RPAR {
      if (!isDefined($4))
        PARSE_ERROR("unknown symbol value: displacement value for symbol '%s' must be known during assembling", $4);

      if (getSymbolValue($4) >= (1 << 11) || getSymbolValue($4) < -(1 << 11))
        PARSE_ERROR("symbol value too large: displacement for symbol '%s' must fit as a signed value in 12b", $4);

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
  fprintf(stderr, "Parse error (line %d): %s\n", line_num, c);
}