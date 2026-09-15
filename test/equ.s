.equ nesto, 0xf
.equ jos, nesto
.equ normal, nesto + jos
.equ divlje, petar - panpan + petar
.section main
petar: .word 100
.equ pan, 120
    ld $pan, %r2
    ld petar, %r3
panpan: .word 200