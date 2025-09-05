;----------------------------------------------------
; Opcion 1: Ingresar calificaciones (hasta 15)
; - Formato: Nombre Apellido1 Apellido2 Nota
; - Nota: [0..100], entero o decimal con hasta 5 decimales
; - Digitar '9' solo -> regresa al menu
;  - Generado parcialmente con ayuda de IA (ChatGPT).
;----------------------------------------------------
IngresarCalificaciones proc
IngresarLoop:
    call Cls
    mov al, [studentsCount]
    cmp al, 15
    jb  PideLinea
    mov dx, offset msgLleno
    call PrintString
    mov ah,1
    int 21h
    ret

PideLinea:
    mov dx, offset msgIngresar
    call PrintString

    ; Lee línea (AH=0Ah)
    mov dx, offset bufferLinea
    mov ah, 0Ah
    int 21h

    ; '9' + Enter -> regresa
    mov si, offset bufferLinea
    mov bl, [si+1]
    cmp bl, 1
    jne  ValidaYGuarda
    mov al, [si+2]
    cmp al, '9'
    je  RegresarMenu

    ; Verificar 3 palabras (Nombre + 2 apellidos)
    mov si, offset bufferLinea
    mov cl, [si+1]       ; longitud de la línea
    cmp cl, 0
    je  NotaInvalida     ; Si no hay texto
    add si, 2            ; Saltar los 2 bytes de longitud

    mov bl, 0            ; contador de espacios
ContarEspacios:
    cmp cl, 0
    je  VerificarEspacios
    mov al, [si]
    cmp al, ' '
    jne NoEspacio
    inc bl
NoEspacio:
    inc si
    dec cl
    jmp ContarEspacios

VerificarEspacios:
    cmp bl, 2            ; Verificar que haya 2 espacios (es decir, 3 partes)
    jb  NombreInvalido   ; Si no hay 2 espacios, nombre inválido
    ; Sigue flujo normal para procesar la entrada


ValidaYGuarda:
    ; Si es válido, procesar la entrada como nombre y apellido
    mov si, offset bufferLinea
    mov bl, [si+1]         ; longitud
    mov di, si
    add di, 2              ; di -> primer char
    mov cx, bx             ; cx = len
    add di, cx             ; di -> fin (1 char después del último)
    dec di                 ; di -> último char

    ; Saltar espacios finales
SkipTrail:
    cmp cx, 0
    je  NotaInvalida
    cmp byte ptr [di], ' '
    jne BuscaUltimoEsp
    dec di
    dec cx
    jmp SkipTrail

BuscaUltimoEsp:
    mov bx, cx
FindSpaceBack:
    cmp bx, 0
    je  NoSpaceFound
    cmp byte ptr [di], ' '
    je  FoundSpace
    dec di
    dec bx
    jmp FindSpaceBack
FoundSpace:
    inc di                 ; di / inicio de la nota
    jmp TocaValidar
NoSpaceFound:
    mov di, si
    add di, 2

TocaValidar:
    ; Validación y rango
    mov bp, 0                  ; 0 = antes de '.', 1 = después
    mov byte ptr decCount, 0   ; # de decimales
    mov byte ptr fracNZ, 0     ; hubo decimal no-cero
    mov word ptr intVal, 0     ; parte entera acumulada (0..100)

    ; --- fracción en 32 bits ---
    mov word ptr fracLo, 0
    mov word ptr fracHi, 0


ValLoop:
    ; detener al final de la linea
    mov si, offset bufferLinea
    mov al, [si+1]
    xor ah, ah
    add si, 2
    add si, ax                 
    cmp di, si
    jae FinVal

    mov al, [di]

    ; digito?
    cmp al, '0'
    jb  CheckDot
    cmp al, '9'
    ja  CheckDot

    ; Si es digito
    cmp bp, 0
    jne AfterDotDigit

    ; Parte entera: intVal = intVal*10 + dígito
    mov  ax, [intVal]
    mov  bx, 10
    mul  bx                 ; DX:AX = intVal*10   (intVal ≤ 100, cabe)
    xor  bh, bh
    mov  bl, [di]
    sub  bl, '0'            ; BL = dígito 0..9
    add  ax, bx
    mov  [intVal], ax
    cmp  ax, 100
    ja   NotaFueraRango
    jmp  NextChar


AfterDotDigit:
    inc  byte ptr decCount
    cmp  byte ptr decCount, 5
    ja   NotaInvalida

    ; --- guardar el dígito (0..9) en la pila ---
    xor  ah, ah
    mov  al, [di]
    sub  al, '0'          ; AL = dígito 0..9
    push ax               ; [stack] ... , digit

    ; --- vamos a usar varios registros, salvamos ---
    push bx
    push dx
    push si
    push di

    ; --- frac(32) = frac(32) * 10 ---
    mov  ax, fracLo
    mov  bx, 10
    mul  bx              ; DX:AX = fracLo * 10
    mov  si, ax          ; nuevo low
    mov  di, dx          ; carry del low

    mov  ax, fracHi
    mov  bx, 10
    mul  bx              ; DX:AX = fracHi * 10
    add  ax, di
    adc  dx, 0

    mov  fracLo, si
    mov  fracHi, ax

    ; --- restaurar los regs que salvamos (digit sigue en la pila) ---
    pop  di
    pop  si
    pop  dx
    pop  bx

    ; --- recuperar el dígito y sumarlo: frac += digit ---
    pop  ax              ; AX = digit (0..9)
    add  fracLo, ax
    adc  fracHi, 0

    ; marcar si hubo decimal no-cero
    cmp  al, 0
    je   NextChar
    mov  byte ptr fracNZ, 1
    jmp  NextChar




CheckDot:
    cmp al, '.'
    jne NotaInvalida
    cmp bp, 0
    jne NotaInvalida           ; segundo punto
    mov bp, 1                 

NextChar:
    inc di
    jmp ValLoop

FinVal:
    ; no debe terminar en '.'
    dec di
    cmp byte ptr [di], '.'
    je  NotaInvalida
    inc di

    ; --- Chequeo de rango final ---
    mov ax, [intVal]
    cmp ax, 100
    jb  RangoOK                ; <100 siempre ok
    ja  NotaFueraRango         ; >100 no
    ; si es 100 solo decimales 0
    cmp bp, 0
    je  RangoOK                ; 100 (entero)
    cmp byte ptr decCount, 0
    je  RangoOK                ; 100.
    cmp byte ptr fracNZ, 0
    jne NotaFueraRango         ; 100.x con x!=0    
    
RangoOK:
    ; ============================
    ; 1) intVal * 100000
    ; ============================
    ; primero intVal * 10000
    mov  ax, [intVal]       ; 0..100
    mov  bx, 086A0h         ; 34464
    mul  bx                 ; DX:AX = intVal * 34464   (32-bit)
    add  dx, [intVal]       ; + intVal * 65536  => DX:AX = intVal * 100000

    ; guarda ENTERO escalado (para sumarle la fracción luego)
    push dx                 ; intPartHi
    push ax                 ; intPartLo


    ; ============================
    ; 2) fracVal * 10^(5 - decCount)
    ; ============================
    cmp  bp, 0                   ; ¿hubo '.' ?
    jne  DoFracScale
    ; si NO hubo '.', frac ya está en 0 por la init en TocaValidar
    jmp  NoFracScale

DoFracScale:
    ; frac(32) *= 10^(5 - decCount)
    mov  cl, 5
    sub  cl, [decCount]
    jz   NoFracScale

FracScaleLoop:
    ; frac(32) *= 10
    push ax
    push bx
    push dx
    push si
    push di

    mov  ax, fracLo
    mov  bx, 10
    mul  bx              ; DX:AX = fracLo * 10
    mov  si, ax
    mov  di, dx

    mov  ax, fracHi
    mov  bx, 10
    mul  bx              ; DX:AX = fracHi * 10
    add  ax, di
    adc  dx, 0

    mov  fracLo, si
    mov  fracHi, ax

    pop  di
    pop  si
    pop  dx
    pop  bx
    pop  ax

    dec  cl
    jnz  FracScaleLoop

NoFracScale:
    ; ============================
    ; 3) suma: notaEscalada = intPart(32) + frac(32)
    ; ============================

    ; recuperar entero escalado (que dejaste en la pila)
    pop  si               ; intPartLo
    pop  di               ; intPartHi

    ; cargar fracción
    mov  ax, fracLo
    mov  dx, fracHi

    ; nota = entero + fracción  -> DX:AX
    add  ax, si
    adc  dx, di


SumParts:
    ; Guardar notaEscalada en notas32[studentsCount]
    mov bx, 0
    mov bl, [studentsCount]
    shl bx, 2                  
    mov di, offset notas32
    add di, bx
    mov [di], ax               ; low word
    mov [di+2], dx             ; high word

    ; Guardar línea completa 
    mov al, [studentsCount]
    xor ah, ah                 ; AX = idx
    mov bx, ax                 
    mov ax, bx
    shl ax, 4                  ; idx*16
    mov dx, bx
    shl dx, 6                  ; idx*64
    add ax, dx                 
    mov di, offset studentsBuf
    add di, ax                 ; DI = destino del registro

    ; 3.b) Guardar recOfs[idx] = DI
    mov bx, 0
    mov bl, [studentsCount]
    shl bx, 1                  
    mov si, offset recOfs
    add si, bx
    mov [si], di               

    ; Copiar la líneas
    mov si, offset bufferLinea
    mov cl, [si+1]             ; longitud real
    xor ch, ch
    add si, 2                  ; primer carácter
    cld                
    mov  ax, ds
    mov  es, ax         
    rep movsb

    mov byte ptr [di], 0Dh
    inc di
    mov byte ptr [di], 0Ah
    inc di
    mov byte ptr [di], '$'
    inc di

    ; Incrementar contador
    mov al, [studentsCount]
    inc al
    mov [studentsCount], al

    ; Mensaje y pausa
    mov dx, offset msgOK
    call PrintString
    mov ah,1
    int 21h
    jmp IngresarLoop

NotaFueraRango:
    mov dx, offset msgRangoInv
    call PrintString
    mov ah,1
    int 21h
    jmp IngresarLoop

NotaInvalida:
    mov dx, offset msgNotaInv
    call PrintString
    mov ah,1
    int 21h
    jmp IngresarLoop

NombreInvalido:
    mov dx, offset msgNombreInv
    call PrintString
    mov ah, 1
    int 21h
    jmp IngresarLoop


RegresarMenu:
    ret
IngresarCalificaciones endp  
