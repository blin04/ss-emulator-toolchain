%{
#include "parser.tab.h"
%}
    /* definitions */ 

BIN_INT     0(b|B)[01]*
COMMA       ,
COMMENT     #.*\n
COLON       :
DEC_INT     [0-9]*
DOLLAR      $
DOT         .
HEX_INT     0(x|X)[0-9a-fA-F]*
NL          [\n|(\r\n)]
LITERAL     {BIN_INT}|{OCT_INT}|{DEC_INT}|{HEX_INT}
LPAR        [
OCT_INT     0[0-7]*
PLUS        +
PERCENT     %
RPAR        ]
SYMBOL      ([a-z]|[A-Z]|[_.$])([a-z]|[A-Z]|[_.$]|[0-9])*

    /* instructions */

HALT        halt
INT         int
IRET        iret
CALL        call
RET         ret
JMP         jmp
BEQ         beq
BNE         bne
BGT         bgt
PUSH        push
POP         pop
XCHNG       xchng
ADD         add
SUB         sub
MUL         mul
DIV         div
NOT         not
AND         and
OR          or
XOR         xor
SHL         shl
SHR         shr
LD          ld
ST          st
CSSRD       cssrd  
CSRWR       csrwr

    /* directives */ 

ASCII       ascii
END         end
EQU         equ
EXTERN      extern
GLOBAL      global
SECTION     section
SKIP        skip
WORD        word

    /* registers */ 

CAUSE       cause
REG         r(1[0-5]|[0-9])
HANDLER     handler
SP          sp
STAUTS      status
PC          pc

%%

{HALT}      return HALT;
{SYMBOL}    return SYMBOL;  
{LITERAL}   return LITERAL;
