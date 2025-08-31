PrintString proc
    mov ah,9
    int 21h
    ret
PrintString endp



PrintNumber proc
    push ax
    push bx
    push cx
    push dx

    xor cx, cx        ; contador de dígitos
    mov bx, 10

PN_Loop:
    xor dx, dx
    div bx            ; AX / 10 -> AX=cociente, DX=resto
    push dx           ; guardar dígito en la pila
    inc cx
    cmp ax, 0
    jne PN_Loop

PN_Print:
    pop dx
    add dl, '0'       ; convertir a ASCII
    mov ah, 02h
    int 21h
    loop PN_Print

    pop dx
    pop cx
    pop bx
    pop ax
    ret
PrintNumber endp

Cls proc
    mov ax,0600h
    mov bh,07h
    mov cx,0000h
    mov dx,184Fh
    int 10h
    mov ah,02h
    mov bh,0
    mov dx,0000h
    int 10h
    ret
Cls endp

CRLF proc
    mov ah,02h
    mov dl,0Dh
    int 21h
    mov dl,0Ah
    int 21h
    ret
CRLF endp

; (opcional) PrintNumDec aquí
