.section main
.global func
.extern extern_symbol1
func:
    ld $extern_symbol1, %r1
symbol: .word 0xff
    ld undef_symbol, %r2
    ld [%r1 + symbol], %r3
    ld [%r8 + 2047], %r10
    ld $3000, %r5
 
undef_symbol: .word 1000
 
    ld moze_mi_se, %r10
    div %r10, %r6
    st %r10, moze_mi_se
    .word 0xabcdef
    .word 0x11223344