.section main
simbol:
.word dram, bejs, hip, hop, rege 
novi_simbol: .word 123, 456, 0xffff 
jos_jedan_simbol:
.global simbol, novi_simbol
msg_start:
.ascii "ovo je string koji ce da se vidi lesgo"
msg_end:
.equ msg_len, msg_end - msg_start

