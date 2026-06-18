%{
  #include <stdio.h>

  int yylex();
  void yyerror(const char *);
%}

%union {

}

/* declaring used tokens */

%token HALT INT IRET CALL RET JMP BEQ BNE 
%token BGT PUSH POP XCHNG ADD SUB MUL DIV 
%token NOT AND OR XOR SHL SHR LD ST CSSRD CSRWR
%token BIN_INT COMMA COMMENT COLON DEC_INT DOLLAR DOT HEX_INT 
%token LITERAL LPAR NL OCT_INT PLUS PERCENT RPAR SYMBOL
%token ASCII END EQU EXTERN GLOBAL SECTION SKIP WORD
%token CAUSE HANDLER PC REG SP STATUS 

%%

/* grammar rules */

// a program is made up of zero or more lines
program: 
    /* empty */
  | program line
  ;

// a line is an instruction statement, a directive, 
// a comment or just an empty line
line:
    DOT directive NL
  | statement NL
  | label NL
  | label DOT directive NL
  | label statement NL
  | COMMENT
  | NL
  ;
  
label:
  SYMBOL COLON
  ;
  
directive:
    DOT ASCII
  | DOT END 
  | DOT EQU 
  | DOT EXTERN 
  | DOT GLOBAL 
  | DOT SECTION 
  | DOT SKIP 
  | DOT WORD     
  ;

statement: 
    zero_op_instr
  | one_op_instr
  | two_op_instr PERCENT gpr COMMA PERCENT gpr
  | three_op_instr PERCENT gpr COMMA PERCENT gpr COMMA jump_operand
  | LD data_operand COMMA PERCENT gpr
  | ST PERCENT gpr COMMA data_operand
  | CSSRD PERCENT csr COMMA PERCENT gpr
  | CSRWR PERCENT gpr COMMA PERCENT csr
  ;

zero_op_instr: 
    HALT 
  | INT 
  | IRET 
  | RET 
  ;

one_op_instr: 
    data_one_op_instr PERCENT gpr 
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
  printf("Parse error: \n");
}