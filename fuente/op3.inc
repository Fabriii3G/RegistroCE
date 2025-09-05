;----------------------------------------------------
; Opcion 3: Buscar estudiante por posicion (indice)
;   - Pide un numero (1..studentsCount)
;   - Imprime el registro si existe
;----------------------------------------------------
BuscarPorIndice proc
    call Cls

    ; Hay estudiantes?
    mov  al, [studentsCount]
    cmp  al, 0
    jne  PreguntaIndice
    mov  dx, offset msgSinDatos
    call PrintString
    mov  ah, 1
    int  21h
    ret

PreguntaIndice:
    mov  dx, offset msgPedirIndice
    call PrintString

    ; Leer línea 
    mov  dx, offset bufferNum
    mov  ah, 0Ah
    int  21h

    ; CRLF
    mov  ah, 02h
    mov  dl, 0Dh
    int  21h
    mov  dl, 0Ah
    int  21h

    ; Validar que hay al menos 1 char
    mov  si, offset bufferNum
    mov  bl, [si+1]              ; longitud
    cmp  bl, 0
    je   IndiceInvalido

    ; Parsear número decimal 
    xor  ax, ax                  
    mov  di, si
    add  di, 2                   
ParseLoop:
    cmp  bl, 0
    je   FinParse

    mov  dl, [di]
    cmp  dl, '0'
    jb   IndiceNoNumerico
    cmp  dl, '9'
    ja   IndiceNoNumerico

    
    mov  dx, ax
    shl  ax, 1                   
    shl  dx, 3                   
    add  ax, dx                  
    mov  cl, [di]
    sub  cl, '0'
    xor  ch, ch
    add  ax, cx

    inc  di
    dec  bl
    jmp  ParseLoop
FinParse:

    ; Rango: 1t
    cmp  ax, 1
    jb   IndiceFueraRango
    mov  bl, [studentsCount]
    xor  bh, bh
    cmp  ax, bx
    ja   IndiceFueraRango

    ; ubicar el registro actual
    dec  ax                        ; idx-1
    shl  ax, 1                     
    mov  si, offset recOfs
    add  si, ax
    mov  di, word ptr [si]         ; DI = offset del registro en studentsBuf

    ; Imprimir el registro 
    mov  dx, di
    call PrintString

    ; Pausa
    mov  ah, 1
    int  21h
    ret

IndiceNoNumerico:
    mov  dx, offset msgNoNumerico
    call PrintString
    mov  ah, 1
    int  21h
    ret

IndiceFueraRango:
IndiceInvalido:
    mov  dx, offset msgIndiceInv
    call PrintString
    mov  ah, 1
    int  21h
    ret
BuscarPorIndice endp
