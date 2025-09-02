;====================================================
; Opcion 1: Ingresar calificaciones (hasta 15)
; - Formato: Nombre Apellido1 Apellido2 Nota
; - Nota: [0..100], entero o decimal con hasta 5 decimales
; - Digitar '9' solo -> regresa al men?
;====================================================
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

    ; Leer linea (AH=0Ah)
    mov dx, offset bufferLinea
    mov ah, 0Ah
    int 21h

    ; '9' + Enter -> regresar
    mov si, offset bufferLinea
    mov bl, [si+1]
    cmp bl, 1
    jne  ValidaYGuarda
    mov al, [si+2]
    cmp al, '9'
    je  RegresarMenu

    ; --- Verificar que haya al menos 3 palabras (Nombre + 2 apellidos) ---
    mov si, offset bufferLinea
    mov cl, [si+1]       ; longitud de la línea
    cmp cl, 0
    je  NotaInvalida
    add si, 2            ; SI -> primer carácter

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
    cmp bl, 3            ; al menos 3 espacios => 4 palabras
    jb  NombreInvalido   ; si < 2 espacios, error
    ; sigue flujo normal



ValidaYGuarda:
    ; si = buffer base
    mov si, offset bufferLinea
    mov bl, [si+1]         ; len
    mov di, si
    add di, 2              ; di -> primer char
    mov cx, bx             ; cx = len
    add di, cx             ; di -> fin (1 char despues del ultimo)
    dec di                 ; di -> ultimo char

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
    inc di                 ; di -> inicio de la nota
    jmp TocaValidar
NoSpaceFound:
    mov di, si
    add di, 2

TocaValidar:
    ; --- Inicializaciones para validacion y rango ---
    mov bp, 0                  ; 0 = antes de '.', 1 = despues
    mov byte ptr decCount, 0   ; # de decimales
    mov byte ptr fracNZ, 0     ; hubo decimal no-cero
    mov word ptr intVal, 0     ; parte entera acumulada (0..100)   
    mov word ptr fracVal, 0 

ValLoop:
    ; detener al final de la linea
    mov si, offset bufferLinea
    mov al, [si+1]
    xor ah, ah
    add si, 2
    add si, ax                 ; si -> fin+1
    cmp di, si
    jae FinVal

    mov al, [di]

    ; digito?
    cmp al, '0'
    jb  CheckDot
    cmp al, '9'
    ja  CheckDot

    ; --- Es digito ---
    cmp bp, 0
    jne AfterDotDigit

    ; Antes del punto: acumular parte entera y chequear >100
    mov bl, al
    sub bl, '0'                 ; BL = valor [0..9]
    mov ax, [intVal]
    mov bx, 10
    mul bx                      ; DX:AX = intVal*10
    add ax, bx                  ; OJO: BX ahora no es 10; debemos restaurar BL
    ; Correccion: rehacer suma con BL
    ; (ajuste seguro, usa temporales peque?os)
AfterMulFix:
    ; Rehacer la suma correctamente:
    mov ax, [intVal]
    mov bx, 10
    mul bx                      ; DX:AX = intVal*10
    xor bh, bh
    mov bl, [di]
    sub bl, '0'
    add ax, bx                  ; AX = intVal*10 + d?gito
    mov [intVal], ax
    cmp ax, 100
    ja  NotaFueraRango
    jmp NextChar

AfterDotDigit:
    inc byte ptr decCount
    cmp byte ptr decCount, 5
    ja  NotaInvalida

    ; fracVal = fracVal*10 + (al - '0')
    mov bx, 10
    mov ax, [fracVal]
    mul bx                ; DX:AX = fracVal*10
    xor bh, bh
    mov bl, [di]
    sub bl, '0'
    add ax, bx
    mov [fracVal], ax

    cmp al, '0'
    je  NextChar
    mov byte ptr fracNZ, 1
    jmp NextChar


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
    ; ==100: solo v?lido si no hay decimales no-cero
    cmp bp, 0
    je  RangoOK                ; 100 (entero)
    cmp byte ptr decCount, 0
    je  RangoOK                ; 100.
    cmp byte ptr fracNZ, 0
    jne NotaFueraRango         ; 100.x con x!=0    
    
RangoOK:
    ; ===== 1) Construir notaEscalada = intVal*100000 + fracVal * 10^(5 - decCount) =====
    ; 1.a) intVal * 100000  (usando *10000 y luego *10)
    mov ax, [intVal]
    mov bx, 10000
    mul bx              ; DX:AX = intVal * 10000  (32-bit)

    ; *10 = (<<3) + (<<1)
    push dx
    push ax
    pop ax
    pop dx
    mov si, ax          ; guardar original lo
    mov di, dx          ; guardar original hi

    ; resultado = original << 1
    shl ax, 1
    rcl dx, 1
    push dx             ; guardar (<<1) hi
    push ax             ; guardar (<<1) lo

    ; volver al original en SI:DI y hacer <<3
    mov ax, si
    mov dx, di
    shl ax, 1
    rcl dx, 1
    shl ax, 1
    rcl dx, 1
    shl ax, 1
    rcl dx, 1          ; (<<3) en DX:AX

    ; sumar (<<1)
    pop si              ; (<<1) lo
    pop di              ; (<<1) hi
    add ax, si
    adc dx, di          ; DX:AX = intVal*100000
    push dx             ; guarda intPartHi
    push ax             ; guarda intPartLo

    ; 1.b) fracVal * 10^(5 - decCount)  --- versión con LOOP ---
    xor  dx, dx
    mov  ax, [fracVal]          ; DX:AX = fracVal
    xor  cx, cx
    mov  cl, 5
    sub  cl, [decCount]         ; CL = 5 - decCount
    jz   NoFracScale
FracScaleLoop:
    ; (DX:AX) *= 10  = (<<3)+(<<1)
    ; Guardar original en SI:DI
    mov  si, ax
    mov  di, dx

    ; resultado = original << 1
    shl  ax, 1
    rcl  dx, 1
    push dx
    push ax                     ; pila: (<<1)

    ; volver al original y hacer <<3
    mov  ax, si
    mov  dx, di
    shl  ax, 1
    rcl  dx, 1
    shl  ax, 1
    rcl  dx, 1
    shl  ax, 1
    rcl  dx, 1                  ; (<<3) en DX:AX

    ; sumar (<<1)
    pop  si                     ; (<<1) lo
    pop  di                     ; (<<1) hi
    add  ax, si
    adc  dx, di

    loop FracScaleLoop
NoFracScale:
    ; DX:AX = fracScaled
    ; 1.c) Sumar ambas partes ? notaEscalada
    pop si              ; intPartLo
    pop di              ; intPartHi
    add ax, si
    adc dx, di          ; DX:AX = notaEscalada (32-bit)

    ; ===== 2) Guardar notaEscalada en notas32[studentsCount] =====
    mov bx, 0
    mov bl, [studentsCount]
    shl bx, 2                  ; *4 bytes (dd)
    mov di, offset notas32
    add di, bx
    mov [di], ax               ; low word
    mov [di+2], dx             ; high word

    ; ===== 3) Guardar línea completa y metadatos =====
    ; 3.a) Calcular DI = &studentsBuf + idx*80
    mov al, [studentsCount]
    xor ah, ah                 ; AX = idx
    mov bx, ax                 ; BX = idx
    mov ax, bx
    shl ax, 4                  ; idx*16
    mov dx, bx
    shl dx, 6                  ; idx*64
    add ax, dx                 ; AX = idx*80
    mov di, offset studentsBuf
    add di, ax                 ; DI = destino del registro

    ; 3.b) Guardar recOfs[idx] = DI
    mov bx, 0
    mov bl, [studentsCount]
    shl bx, 1                  ; *2 bytes (dw)
    mov si, offset recOfs
    add si, bx
    mov [si], di               ; offset de inicio del registro

    ; 3.c) Copiar la línea desde bufferLinea
    mov si, offset bufferLinea
    mov cl, [si+1]             ; longitud real
    xor ch, ch
    add si, 2                  ; SI -> primer carácter
    rep movsb

    ; 3.d) Terminar con CR, LF, '$'
    mov byte ptr [di], 0Dh
    inc di
    mov byte ptr [di], 0Ah
    inc di
    mov byte ptr [di], '$'
    inc di

    ; 3.e) Incrementar contador
    mov al, [studentsCount]
    inc al
    mov [studentsCount], al

    ; 3.f) Mensaje y pausa
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
