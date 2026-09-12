.section code
    ld value, %r3
    ld $0x1234, %r1
    st %r1, value
    ld value, %r2
    halt
.section data
value: .word 0xffffffff
