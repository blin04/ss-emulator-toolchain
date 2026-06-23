%{
  #include <stdio.h>
  #include <string.h>
  
  #include "../inc/interface.h"

  int yylex();
  void yyerror(const char *);

  int line_num = 0;
  int location_counter = 0;

  int allSymbolsDefined(char** symbs) {
    for (int i = 0; symbs[i] != NULL; i++) {
      if (!isDefined(symbs[i])) {
        // todo: add symbol name to message
        yyerror("symbol not defined");
        return 0;
      }
    } 
    return 1;
  }

%}

%union {
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

%type <sval> label;
%type <arrval> symbol_list
%type <arrval> symbol_or_literal_list

%%

/* grammar rules */

program: 
    line
  | program NL line {printf("parsed program\n");}
  ;

line:
    /* empty */ { line_num++; }
  | directive comment { line_num++; addDirective(); }
  | statement comment { line_num++; addInstruction(); }
  | label comment { line_num++; defineSymbol($1, location_counter); }
  | label directive comment { line_num++; defineSymbol($1, location_counter); addDirective(); }
  | label statement comment { line_num++; defineSymbol($1, location_counter); addInstruction(); }
  | COMMENT { line_num++; }
  ;

comment:
    /* empty */
  | COMMENT
  
label:
  SYMBOL COLON { $$ = $1; }
  ;
  
directive:
    ASCII STRING {  addAsciiDirective($2); location_counter += strlen($2); free($2); }
  | END { YYACCEPT; /* end parsing successfully */ }
  | EQU SYMBOL COMMA exp 
  | EXTERN symbol_list { declareSymbolsExtern($2); }
  | GLOBAL symbol_list { if (!allSymbolsDefined($2)) { YYERROR; } declareSymbolsGlobal($2); }
  | SECTION SYMBOL { startNewSection($2); }
  | SKIP LITERAL { addSkipDirective($2); location_counter += $2;}
  | WORD symbol_or_literal_list { 
      addWordDirective($2); 
      for (int i = 0; $2[i] != NULL; i++) location_counter += 4;
    }
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
    exp PLUS exp
  | exp MINUS exp
  | SYMBOL {}
  | LITERAL {}

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