%{
  #include <stdio.h>
  
  #include "../inc/interface.h"

  int yylex();
  void yyerror(const char *);

  int line_num = 0;
%}

%union {
  long lval;
  char* sval;
}

/* declaring used tokens */

%token HALT INT IRET CALL RET JMP BEQ BNE 
%token BGT PUSH POP XCHNG ADD SUB MUL DIV 
%token NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token COMMA COMMENT COLON DOLLAR <lval> LITERAL
%token LPAR MINUS NL PLUS RPAR <sval> SYMBOL STRING
%token ASCII END EQU EXTERN GLOBAL SECTION SKIP WORD
%token CAUSE HANDLER PC REG SP STATUS 

%left PLUS MINUS

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
  | label comment { line_num++; defineSymbol(); }
  | label directive comment { line_num++; declareSymbolGlobal(); }
  | label statement comment { line_num++; declareSymbolExtern(); }
  | COMMENT { line_num++; }
  ;

comment:
    /* empty */
  | COMMENT
  
label:
  SYMBOL COLON
  ;
  
directive:
    ASCII STRING
  | END { YYACCEPT; /* end parsing successfully */ }
  | EQU SYMBOL COMMA exp 
  | EXTERN symbol_list
  | GLOBAL symbol_list
  | SECTION SYMBOL {
      startNewSection($2);
    }
  | SKIP LITERAL {
      addSkipDirective($2);
    }
  | WORD symbol_or_literal_list
  ;

symbol_list:
    SYMBOL
  | symbol_list COMMA SYMBOL
  ;

symbol_or_literal_list:
    SYMBOL
  | LITERAL
  | symbol_or_literal_list COMMA SYMBOL
  | symbol_or_literal_list COMMA LITERAL


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
  | SYMBOL
  | LITERAL

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
  printf("Parse error: token value %s, at line number %d \n", yylval, line_num);
}