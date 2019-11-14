global printKKP
global printStr
SECTION .data
msg db "KKP!!", 0ah
msglen equ ($ - msg)
SECTION .bss
SECTION .text

printStr:
    ;准备阶段
    push ebp
    mov ebp, esp
    push eax;字符串首地址，[ebp - 4]
    push ebx
    push ecx
    push edx

    ;过程体
    mov eax, [ebp + 8]
    mov ecx, eax
    push ecx
    call strLength;eax为参数（地址），返回值放在edx。
    pop ecx
    mov eax, 4d
    mov ebx, 1d
    int 80h

    ;结束阶段
    pop edx
    pop ecx
    pop ebx
    pop eax 
    leave
    ret
strLength:
    ;准备阶段
    push ebp
    mov ebp, esp
    push eax;[ebp - 4]
    push ebx
    push ecx
    push edx

    ;过程体
    mov edx, 0d
strLengthLoop:
    mov ecx, eax
    add ecx, edx;ecx = eax + edx(地址+偏移量)
    mov cl, byte[ecx];取一个byte
    cmp cl, 0
    je strLengthEnd
    add edx, 1
    jmp strLengthLoop

strLengthEnd:
    ;结束阶段
    sub esp, 4 
    pop ecx
    pop ebx
    pop eax 
    leave
    ret