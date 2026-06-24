.section main
func:
    halt
    iret
    int
    ret
loop: not %r0
    push %r1
    pop %r2
    jmp loop
    call func
    jmp 0x20
    add %r3, %r4
    div %r2, %r5
    sub %r3, %r7
    mul %r6, %r9
    csrrd %handler, %r5
    csrwr %r10, %cause