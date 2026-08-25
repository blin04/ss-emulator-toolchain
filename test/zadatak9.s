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
ld %r8, e
.section data
.skip 8
e: .word a-e
.word c
.word .bss
a: .word b
.section bss
c: .skip 8
.end