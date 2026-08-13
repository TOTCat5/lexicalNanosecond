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
mov ebx,[ebp+8]
mov ecx,[ebp+12]
sub ebx,ecx
mov [ebp-0],ebx
mov eax, [ebp-0]
mov esp,ebp
pop ebp
ret
