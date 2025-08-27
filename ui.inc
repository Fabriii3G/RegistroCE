PrintString proc
    mov ah,9
    int 21h
    ret
PrintString endp

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
