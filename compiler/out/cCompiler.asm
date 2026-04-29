global _start
extern ExitProcess
section .text
_start:
    call main
    mov rcx,rax
    sub rsp,40
    call ExitProcess
    add rsp,40
main:
    xor rax,rax
    ret