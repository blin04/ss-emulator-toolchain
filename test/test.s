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