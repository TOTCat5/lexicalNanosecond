addition:
    push ebp
    mov ebp,esp
    sub esp,4
    mov ebx, [ebp+8]
    mov ecx, [ebp+12]
    add ebx, ecx
    mov [ebp-0], ebx
    mov eax, [ebp-0]
    mov esp,ebp
    pop ebp
    ret
substaction:
    push ebp
    mov ebp,esp
    sub esp,4
    sub esp,4
    mov ebx,[ebp+12]
    mov [esp+4],ebx
    sub esp,4
    mov ebx,[ebp+16]
    mov [esp+4],ebx
    call addition
    add esp,8
    mov [ebp-0],eax
    mov eax, [ebp-0]
    mov esp,ebp
    pop ebp
    ret
