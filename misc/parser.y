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
%token NOT AND OR XOR SHL SHR LD ST CSSRD CSRWR
%token COMMA COMMENT COLON DOLLAR DOT LITERAL
%token LPAR NL PLUS PERCENT RPAR SYMBOL
%token ASCII END EQU EXTERN GLOBAL SECTION SKIP WORD
%token CAUSE HANDLER PC REG SP STATUS 

%%

/* grammar rules */

// a program is made up of zero or more lines
program: 
    line
  | program NL line {printf("parsed program\n");}
  ;

// a line is an instruction statement, a directive, 
// a comment or just an empty line
line:
    /* empty */ { line_num++; }
  | directive { line_num++; printf("parsed directive\n"); }
  | statement { line_num++; }
  | label { line_num++; }
  | label DOT directive { line_num++; }
  | label statement { line_num++; }
  | COMMENT { line_num++; }
  ;
  
label:
  SYMBOL COLON
  ;
  
directive:
    ASCII
  | END 
  | EQU 
  | EXTERN symbol_list {printf("parsed extern directive\n");}
  | GLOBAL symbol_list
  | SECTION 
  | SKIP 
  | WORD     
  ;

symbol_list:
  SYMBOL
  | symbol_list COMMA SYMBOL
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
  printf("Parse error: token value %s, at line number %d \n", yylval, line_num);
}