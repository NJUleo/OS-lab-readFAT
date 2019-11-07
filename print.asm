SECTION .data
msg db "KKP!!", 0ah
msglen equ ($ - msg)
SECTION .bss
SECTION .text
global _start

_start:
    mov eax, 4d
    mov ebx, 1d
    mov ecx, msg
    mov edx, msglen
    int 80h

    mov ebx, 0d
    mov eax, 1d
    int 80h