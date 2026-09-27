main:
push ebp
mov ebp,esp
sub esp,1
mov ebx,[ebp+12]
mov cl,1
cmp ebx,cl
sete ebx
mov [ebp-0],ebx
jmp t1
sub esp,4
mov ebx,1
neg ebx
mov [ebp-1]ebx
mov eax, [ebp-1]
mov esp,ebp
pop ebp
ret
