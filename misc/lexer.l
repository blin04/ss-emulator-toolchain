%{
#include "parser.tab.h"
#include <stdio.h>
%}

%option noyywrap


/* instructions */

ADD         add
AND         and
BEQ         beq
BGT         bgt
BNE         bne
CALL        call
CSSRD       cssrd
CSRWR       csrwr
DIV         div
HALT        halt
INT         int
IRET        iret
JMP         jmp
LD          ld
MUL         mul
NOT         not
OR          or
POP         pop
PUSH        push
RET         ret
SHL         shl
SHR         shr
ST          st
SUB         sub
XCHNG       xchng
XOR         xor

/* directives */ 

ASCII       .ascii
END         .end
EQU         .equ
EXTERN      .extern
GLOBAL      .global
SECTION     .section
SKIP        .skip
WORD        .word

/* registers */ 

CAUSE       cause
REG         r(1[0-5]|[0-9])
HANDLER     handler
SP          sp
STAUTS      status
PC          pc

/* definitions */ 

BIN_INT     0(b|B)[01]*
COMMA       ,
COMMENT     #.*\n
COLON       :
DEC_INT     [0-9]*
DOLLAR      $
DOT         \.
HEX_INT     0(x|X)[0-9a-fA-F]*
NL          [\n|(\r\n)]
LITERAL     {BIN_INT}|{OCT_INT}|{DEC_INT}|{HEX_INT}
LPAR        [
OCT_INT     0[0-7]*
PLUS        +
PERCENT     %
RPAR        ]
SYMBOL      ([a-z]|[A-Z]|[_.$])([a-z]|[A-Z]|[_.$]|[0-9])*

%%

{ADD}       return ADD;
{AND}       return AND;
{BEQ}       return BEQ;
{BGT}       return BGT;
{BNE}       return BNE;
{CALL}      return CALL;
{CSSRD}     return CSSRD;
{CSRWR}     return CSRWR;
{DIV}       return DIV;
{HALT}      return HALT;
{INT}       return INT;
{IRET}      return IRET;
{JMP}       return JMP;
{LD}        return LD;
{MUL}       return MUL;
{NOT}       return NOT;
{NL}        return NL;
{OR}        return OR;
{POP}       return POP;
{PUSH}      return PUSH;
{RET}       return RET;
{SHL}       return SHL;
{SHR}       return SHR;
{ST}        return ST;
{SUB}       return SUB;
{XCHNG}     return XCHNG;
{XOR}       return XOR;
{ASCII}     return ASCII;
{END}       {printf("found end\n"); return END;}
{EQU}       return EQU;
{EXTERN}    {printf("found extern\n"); return EXTERN;}
{GLOBAL}    return GLOBAL;
{SECTION}   return SECTION;
{SKIP}      return SKIP;
{WORD}      return WORD;
{CAUSE}     return CAUSE;
{REG}       return REG;
{HANDLER}   return HANDLER;
{SP}        return SP;
{STAUTS}    return STATUS;
{PC}        return PC;
{COMMA}     return COMMA;
{DOT}       {printf("found dot\n"); return DOT;}
{COMMENT}   return COMMENT;
{COLON}     return COLON;
{SYMBOL}    {printf("found symbol\n"); return SYMBOL; } 
{LITERAL}   return LITERAL;
