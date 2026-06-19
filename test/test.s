.extern isr_timer, isr_terminal

.global handler
.section my_handler
handler:
    push %r1
    push %r2
    csrrd %cause, %r1