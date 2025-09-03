;----------------------------------------------------
; Opcion 3: Buscar estudiante por posicion (indice)
;   - Pide un numero (1..studentsCount)
;   - Imprime el registro si existe
;   - Usa recOfs[(idx-1)] para respetar el orden actual
;----------------------------------------------------
BuscarPorIndice proc
    call Cls

    ; ¿Hay estudiantes?
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

    ; Leer línea corta con AH=0Ah (máx 3 dígitos)
    mov  dx, offset bufferNum
    mov  ah, 0Ah
    int  21h

    ; CRLF
    mov  ah, 02h
    mov  dl, 0Dh
    int  21h
    mov  dl, 0Ah
    int  21h

    ; Validar que haya al menos 1 char
    mov  si, offset bufferNum
    mov  bl, [si+1]              ; longitud
    cmp  bl, 0
    je   IndiceInvalido

    ; Parsear número decimal en AX (versión sin LOOP)
    xor  ax, ax                  ; AX = valor
    mov  di, si
    add  di, 2                   ; DI -> primer carácter
ParseLoop:
    cmp  bl, 0
    je   FinParse

    mov  dl, [di]
    cmp  dl, '0'
    jb   IndiceNoNumerico
    cmp  dl, '9'
    ja   IndiceNoNumerico

    ; ax = ax*10 + (dl - '0')
    mov  dx, ax
    shl  ax, 1                   ; *2
    shl  dx, 3                   ; *8
    add  ax, dx                  ; *10
    mov  cl, [di]
    sub  cl, '0'
    xor  ch, ch
    add  ax, cx

    inc  di
    dec  bl
    jmp  ParseLoop
FinParse:

    ; Rango: 1..studentsCount
    cmp  ax, 1
    jb   IndiceFueraRango
    mov  bl, [studentsCount]
    xor  bh, bh
    cmp  ax, bx
    ja   IndiceFueraRango

    ; Usar recOfs[(idx-1)] para ubicar el registro actual (ordenado)
    dec  ax                        ; idx-1
    shl  ax, 1                     ; (idx-1)*2 (cada recOfs es dw)
    mov  si, offset recOfs
    add  si, ax
    mov  di, word ptr [si]         ; DI = offset del registro en studentsBuf

    ; Imprimir el registro (termina en '$')
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
