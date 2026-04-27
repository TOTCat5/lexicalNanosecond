global _start
extern ExitProcess
section .text
_start:
    sub rsp,40
    call main
    mov rcx,rax
    call ExitProcess
    add rsp,40
main:
    xor rax,rax
    ret