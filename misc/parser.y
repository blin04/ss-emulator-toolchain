%{
  #include <stdio.h>

  int yylex();
  void yyerror(const char *);

  int line_num = 0;
%}

%union {

}

/* declaring used tokens */

%token HALT INT IRET CALL RET JMP BEQ BNE 
%token BGT PUSH POP XCHNG ADD SUB MUL DIV 
%token NOT AND OR XOR SHL SHR LD ST CSRRD CSRWR
%token COMMA COMMENT COLON DOLLAR DOT LITERAL
%token LPAR NL PLUS PERCENT RPAR SYMBOL STRING
%token ASCII END EQU EXTERN GLOBAL SECTION SKIP WORD
%token CAUSE HANDLER PC REG SP STATUS 

%%

/* grammar rules */

program: 
    line
  | program NL line {printf("parsed program\n");}
  ;

line:
    /* empty */ { line_num++; }
  | directive comment { line_num++; printf("parsed directive\n"); }
  | statement comment { line_num++; }
  | label comment { line_num++; }
  | label directive comment { line_num++; }
  | label statement comment { line_num++; }
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
  | END 
  | EQU   // will be done later
  | EXTERN symbol_list {printf("parsed extern directive\n");}
  | GLOBAL symbol_list
  | SECTION SYMBOL
  | SKIP LITERAL
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
  | two_op_instr gpr COMMA gpr {printf("parsed two operand instruciton\n"); }
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
  | ADD { printf("parsed add\n"); }
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
  | PERCENT gpr
  | LPAR PERCENT gpr RPAR
  | LPAR PERCENT gpr PLUS LITERAL RPAR
  | LPAR PERCENT gpr PLUS SYMBOL RPAR

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