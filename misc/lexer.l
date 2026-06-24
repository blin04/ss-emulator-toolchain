%{
#include <stdio.h>
#include <string.h>

#include "parser.tab.h"
%}

%option noyywrap


/* statements */

ADD         add
AND         and
BEQ         beq
BGT         bgt
BNE         bne
CALL        call
CSRRD       csrrd
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

CAUSE       %cause
REG         %r(1[0-5]|[0-9])
HANDLER     %handler
SP          %sp
STAUTS      %status
PC          %pc

/* definitions */ 

BIN_INT     0(b|B)[01]*
COMMA       ,
COMMENT     #.*
COLON       :
DEC_INT     [0-9]*
DOLLAR      \$
HEX_INT     0(x|X)[0-9a-fA-F]*
LITERAL     {BIN_INT}|{OCT_INT}|{DEC_INT}|{HEX_INT}
LPAR        \[
MINUS       -
NL          [\n|(\r\n)]
OCT_INT     0[0-7]*
PLUS        \+
RPAR        \]
STRING      \".*\"
SYMBOL      ([a-z]|[A-Z]|[_.$])([a-z]|[A-Z]|[_.$]|[0-9])*

%%

{ADD}       { yylval.ival = ADD; return ADD; }
{AND}       { yylval.ival = AND; return AND; }
{BEQ}       { yylval.ival = BEQ; return BEQ; }
{BGT}       { yylval.ival = BGT; return BGT; }
{BNE}       { yylval.ival = BNE; return BNE; }
{CALL}      { yylval.ival = CALL; return CALL; }
{CSRRD}     { yylval.ival = CSRRD; return CSRRD; }
{CSRWR}     { yylval.ival = CSRWR; return CSRWR; }
{DIV}       { yylval.ival = DIV; return DIV; }
{HALT}      { yylval.ival = HALT; return HALT; }
{INT}       { yylval.ival = INT; return INT; }
{IRET}      { yylval.ival = IRET; return IRET; }
{JMP}       { yylval.ival = JMP; return JMP; }
{LD}        { yylval.ival = LD; return LD; }
{MUL}       { yylval.ival = MUL; return MUL; }
{NOT}       { yylval.ival = NOT; return NOT; }
{NL}        return NL;
{OR}        { yylval.ival = OR; return OR; }
{POP}       { yylval.ival = POP; return POP; }
{PUSH}      { yylval.ival = PUSH; return PUSH; }
{RET}       { yylval.ival = RET; return RET; }
{SHL}       { yylval.ival = SHL; return SHL; }
{SHR}       { yylval.ival = SHR; return SHR; }
{ST}        { yylval.ival = ST; return ST; }
{SUB}       { yylval.ival = SUB; return SUB; }
{XCHNG}     { yylval.ival = XCHNG; return XCHNG; }
{XOR}       { yylval.ival = XOR; return XOR; }
{ASCII}     return ASCII;
{END}       return END;
{EQU}       return EQU;
{EXTERN}    return EXTERN;
{GLOBAL}    return GLOBAL;
{SECTION}   return SECTION;
{SKIP}      return SKIP;
{WORD}      return WORD;
{CAUSE}     return CAUSE;
{REG}       { yylval.ival = atoi(yytext + 2); return REG; }
{HANDLER}   return HANDLER;
{SP}        return SP;
{STAUTS}    return STATUS;
{PC}        return PC;
{COMMA}     return COMMA;
{COLON}     return COLON;
{DOLLAR}    return DOLLAR;
{LPAR}      return LPAR;
{MINUS}     return MINUS;
{PLUS}      return PLUS;
{RPAR}      return RPAR;
{COMMENT}   return COMMENT;
{SYMBOL}    { yylval.sval = strdup(yytext); return SYMBOL; }
{LITERAL}   { 
                if (strlen(yytext) < 2 || yytext[0] != '0')
                    yylval.ival = (int)strtol(yytext, NULL, 10);
                else if (yytext[1] == 'b' || yytext[1] == 'B')
                    yylval.ival = (int)strtol(yytext, NULL, 2); 
                else if (yytext[1] == 'x' || yytext[1] == 'X')
                    yylval.ival = (int)strtol(yytext, NULL, 16); 
                else 
                    yylval.ival = (int)strtol(yytext, NULL, 8); 
                return LITERAL;
            }
{STRING}    { yylval.sval = strdup(yytext); return STRING; }
