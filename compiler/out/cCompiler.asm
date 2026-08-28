addition:
    push ebp
    mov ebp,esp
    sub esp,4
    mov ebx, [ebp+12]
    mov ecx, [ebp+16]
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
    mov ebx,[ebp+16]
    mov [esp+4],ebx
    sub esp,4
    mov ebx,[ebp+20]
    mov [esp+4],ebx
    call addition
    add esp,8
    sub esp,4
    mov ebx,eax
    mov ecx,[ebp+20]
    sub ebx,ecx
    mov [ebp-0],ebx
    mov eax, [ebp-0]
    mov esp,ebp
    pop ebp
    ret
