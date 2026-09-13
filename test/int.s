.section main
    ld $isr, %r10
    csrwr %r10, %handler
    ld symbol, %r1

    int

    ld $0xdead, %r13

.section data
symbol: .word 0xabcdef
.section interrupts
isr:
    ld $0xdeaddead, %r5
    iret