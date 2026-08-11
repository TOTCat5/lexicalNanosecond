add:
push ebp
mov ebp,esp
sub esp,4
mov ebx,[esp+12]
mov ecx,[esp+8]
add ebx,ecx
mov [esp+4],ebx
mov eax, [esp+4]
add esp,4
mov esp,ebp
pop ebp
ret
