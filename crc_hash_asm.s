
section .text

global hashFuncCrc
hashFuncCrc:
    mov rsi, rdi
    mov rdi, 5381
    xor rax, rax

    .crc_loop_start:       
        lodsb
        crc32 rdi, rax
        test rax, rax
        jnz .crc_loop_start

    mov rax, rdi
    ret