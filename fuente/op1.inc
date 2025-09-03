;----------------------------------------------------
; Opcion 1: Ingresar calificaciones (hasta 15)
; - Formato: Nombre Apellido1 Apellido2 Nota
; - Nota: [0..100], entero o decimal con hasta 5 decimales
; - Digitar '9' solo -> regresa al menu
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

    ; Lee linea (AH=0Ah)
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
    mov cl, [si+1]       ; longitud linea
    cmp cl, 0
    je  NotaInvalida
    add si, 2           

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
    cmp bl, 3            
    jb  NombreInvalido   ; Intento de validacion
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
    inc di                 ; di / inicio de la nota
    jmp TocaValidar
NoSpaceFound:
    mov di, si
    add di, 2

TocaValidar:
    ; validacion y rango 
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

    ; Acumular parte entera y revisar 100>
    mov bl, al
    sub bl, '0'                 ; BL = valor [0..9]
    mov ax, [intVal]
    mov bx, 10
    mul bx                      
    add ax, bx                  


AfterMulFix:
    ; Rehacer la suma correctamente:
    mov ax, [intVal]
    mov bx, 10
    mul bx                      
    xor bh, bh
    mov bl, [di]
    sub bl, '0'
    add ax, bx                  
    mov [intVal], ax
    cmp ax, 100
    ja  NotaFueraRango
    jmp NextChar

AfterDotDigit:
    inc byte ptr decCount
    cmp byte ptr decCount, 5
    ja  NotaInvalida

    mov bx, 10
    mov ax, [fracVal]
    mul bx               
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
    ; si es 100 solo decimales 0
    cmp bp, 0
    je  RangoOK                ; 100 (entero)
    cmp byte ptr decCount, 0
    je  RangoOK                ; 100.
    cmp byte ptr fracNZ, 0
    jne NotaFueraRango         ; 100.x con x!=0    
    
RangoOK:
    ; 1) Construir notaEscalada 
    ; 1.a) intVal * 100000 
    mov ax, [intVal]
    mov bx, 10000
    mul bx              ; (32-bit)

    
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

    mov ax, si
    mov dx, di
    shl ax, 1
    rcl dx, 1
    shl ax, 1
    rcl dx, 1
    shl ax, 1
    rcl dx, 1          

    ; sumar (<<1)
    pop si              ; (<<1) lo
    pop di              ; (<<1) hi
    add ax, si
    adc dx, di          ; DX:AX = intVal*100000
    push dx             ; guarda intPartHi
    push ax             ; guarda intPartLo

    ; 1.b) LOOP 
    xor  dx, dx
    mov  ax, [fracVal]          
    xor  cx, cx
    mov  cl, 5
    sub  cl, [decCount]         ; CL = 5 - decCount
    jz   NoFracScale

FracScaleLoop:
    ; Guardar original en SI:DI
    mov  si, ax
    mov  di, dx

    ; resultado = original << 1
    shl  ax, 1
    rcl  dx, 1
    push dx
    push ax                     ; pila

    ; volver al original
    mov  ax, si
    mov  dx, di
    shl  ax, 1
    rcl  dx, 1
    shl  ax, 1
    rcl  dx, 1
    shl  ax, 1
    rcl  dx, 1                  

    ; sumar (<<1)
    pop  si                     
    pop  di                    
    add  ax, si
    adc  dx, di

    loop FracScaleLoop
NoFracScale:
    ; 1.c) Sumar ambas partes / notaEscalada
    pop si              ; intPartLo
    pop di              ; intPartHi
    add ax, si
    adc dx, di          ; notaEscalada (32-bit)

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
