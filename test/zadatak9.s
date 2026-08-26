.global a, c
.extern b
.section text
jmp a
jmp e
jmp b
jmp d
d: .word d
st %r10, b
ld c, %r9
st %r8, e
.section data
.skip 8
e: .word 10
.word c
.word .bss
a: .word b
.section bss
c: .skip 8
.end