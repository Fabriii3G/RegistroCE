org 100h    

start:
    call Cls
    call MostrarMenu
    call LeerOpcion
    jmp  start                 ; bucle principal

;----------------------------------------------------
; MostrarMenu
;----------------------------------------------------
MostrarMenu proc
    mov dx, offset titulo
    call PrintString

    mov dx, offset op1
    call PrintString
    mov dx, offset op2
    call PrintString
    mov dx, offset op3
    call PrintString
    mov dx, offset op4
    call PrintString
    mov dx, offset op5
    call PrintString

    mov dx, offset elegir
    call PrintString
    ret
MostrarMenu endp
        
         
;----------------------------------------------------
; LeerOpcion
;----------------------------------------------------
LeerOpcion proc
    mov ah, 1
    int 21h
    mov opcion, al

    cmp al, '1'
    je  IngresarCalificaciones 
    
    cmp al, '3'
    je  BuscarPorIndice        

    cmp al, '5'
    je  Salir

    mov dx, offset salto
    call PrintString

    mov dl, opcion
    mov ah, 2
    int 21h

    mov dx, offset salto2
    call PrintString
    ret
LeerOpcion endp
       
       
;====================================================
; Opcion 1: Ingresar calificaciones (hasta 15)
; - Formato: Nombre Apellido1 Apellido2 Nota
; - Nota: [0..100], entero o decimal con hasta 5 decimales
; - Digitar '9' solo -> regresa al menú
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
    ; (ajuste seguro, usa temporales pequeños)
AfterMulFix:
    ; Rehacer la suma correctamente:
    mov ax, [intVal]
    mov bx, 10
    mul bx                      ; DX:AX = intVal*10
    xor bh, bh
    mov bl, [di]
    sub bl, '0'
    add ax, bx                  ; AX = intVal*10 + dígito
    mov [intVal], ax
    cmp ax, 100
    ja  NotaFueraRango
    jmp NextChar

AfterDotDigit:
    ; Después del punto: contar decimales (<=5) y marcar si alguno != '0'
    inc byte ptr decCount
    cmp byte ptr decCount, 5
    ja  NotaInvalida
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
    ; ==100: solo válido si no hay decimales no-cero
    cmp bp, 0
    je  RangoOK                ; 100 (entero)
    cmp byte ptr decCount, 0
    je  RangoOK                ; 100.
    cmp byte ptr fracNZ, 0
    jne NotaFueraRango         ; 100.x con x!=0
RangoOK:

    ; Guardar línea completa
    mov al, [studentsCount]
    xor ah, ah
    mov bx, ax                 ; bx = índice
    mov cx, 80                 ; tamaño por registro
    mul cl                     ; ax = idx * 80
    mov di, offset studentsBuf
    add di, ax

    mov si, offset bufferLinea
    mov cl, [si+1]
    xor ch, ch
    add si, 2
    rep movsb

    mov byte ptr [di], 0Dh
    inc di
    mov byte ptr [di], 0Ah
    inc di
    mov byte ptr [di], '$'
    inc di

    mov al, [studentsCount]
    inc al
    mov [studentsCount], al

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

RegresarMenu:
    ret
IngresarCalificaciones endp  



 ;----------------------------------------------------
; Opcion 3: Buscar estudiante por posicion (indice)
;   - Pide un numero (1..studentsCount)
;   - Imprime el registro si existe
;   - Maneja errores: sin datos, no numerico, fuera de rango
;----------------------------------------------------
BuscarPorIndice proc
    call Cls

    ; ¿Hay estudiantes?
    mov al, [studentsCount]
    cmp al, 0
    jne  PreguntaIndice
    mov dx, offset msgSinDatos
    call PrintString
    mov ah,1
    int 21h
    ret

PreguntaIndice:
    mov dx, offset msgPedirIndice
    call PrintString

    ; Leer línea corta con AH=0Ah (máx 3 dígitos)
    mov dx, offset bufferNum
    mov ah, 0Ah
    int 21h  
    
    ; Resultado en siguiente linea
    mov ah, 02h
    mov dl, 0Dh
    int 21h
    mov dl, 0Ah
    int 21h


    ; Validar que haya al menos 1 char
    mov si, offset bufferNum
    mov bl, [si+1]          ; longitud
    cmp bl, 0
    je  IndiceInvalido

    ; Parsear número decimal en AX
    xor ax, ax              ; AX = valor
    mov di, si
    add di, 2               ; DI -> primer carácter
ParseLoop:
    cmp bl, 0
    je  FinParse

    mov dl, [di]          ; leer carácter
    cmp dl, '0'
    jb  IndiceNoNumerico
    cmp dl, '9'
    ja  IndiceNoNumerico

    ; ax = ax*10  (ax*8 + ax*2) usando DX como temporal
    mov dx, ax
    shl ax, 1             ; ax = ax*2
    shl dx, 3             ; dx = ax(original)*8
    add ax, dx            ; ax = ax*10

    ; ax += (digit)
    mov cl, [di]
    sub cl, '0'
    xor ch, ch
    add ax, cx

    inc di
    dec bl
    jmp ParseLoop

FinParse:

    ; Rango: 1..studentsCount
    cmp ax, 1
    jb  IndiceFueraRango
    mov cl, [studentsCount]
    xor ch, ch
    cmp ax, cx
    ja  IndiceFueraRango

    ; Calcular offset del slot: (idx-1) * 80
    dec ax                   ; ax = idx-1
    mov bx, ax               ; n
    mov ax, bx
    shl ax, 4                ; n*16
    mov dx, bx
    shl dx, 6                ; n*64
    add ax, dx               ; ax = n*80

    ; DI = &studentsBuf + ax
    mov di, offset studentsBuf
    add di, ax
    

    ; Imprimir el registro (termina en '$')
    mov dx, di
    call PrintString

    ; Pausa
    mov ah,1
    int 21h
    ret

IndiceNoNumerico:
    mov dx, offset msgNoNumerico
    call PrintString
    mov ah,1
    int 21h
    ret

IndiceFueraRango:
IndiceInvalido:
    mov dx, offset msgIndiceInv
    call PrintString
    mov ah,1
    int 21h
    ret
BuscarPorIndice endp


;----------------------------------------------------
; Utilidades
;----------------------------------------------------
PrintString proc
    mov ah, 9
    int 21h
    ret
PrintString endp

Cls proc
    mov ax, 0600h
    mov bh, 07h
    mov cx, 0000h
    mov dx, 184Fh
    int 10h
    mov ah, 02h
    mov bh, 0
    mov dx, 0000h
    int 10h
    ret
Cls endp    

CRLF proc
    mov ah, 02h
    mov dl, 0Dh
    int 21h
    mov dl, 0Ah
    int 21h
    ret
CRLF endp

;----------------------------------------------------
; Salir
;----------------------------------------------------
Salir:
    mov dx, offset mensajeSalir
    call PrintString
    mov ax, 4C00h
    int 21h

;----------------------
; Datos
;----------------------
titulo        db 0Dh,0Ah, '|RegistroCE|',0Dh,0Ah,'$'
op1           db '1. Ingresar calificaciones',0Dh,0Ah,'$'
op2           db '2. Mostrar estadisticas',0Dh,0Ah,'$'
op3           db '3. Buscar estudiante',0Dh,0Ah,'$'
op4           db '4. Ordenar calificaciones',0Dh,0Ah,'$'
op5           db '5. Salir',0Dh,0Ah,'$'
elegir        db 0Dh,0Ah,'Seleccione una opcion: $'
salto         db 0Dh,0Ah,'Ha elegido: $'
salto2        db 0Dh,0Ah,'Presione una tecla para continuar...',0Dh,0Ah,'$'
mensajeSalir  db 0Dh,0Ah,'Gracias por usar RegistroCE!',0Dh,0Ah,'$'

; --- Opcion 1 ---
msgIngresar   db 0Dh,0Ah,'Por favor ingrese su estudiante (Nombre Apellido1 Apellido2 Nota)',0Dh,0Ah
              db 'O digite 9 para salir al menu principal',0Dh,0Ah,'$'
msgNotaInv    db 0Dh,0Ah,'Nota invalida: formato incorrecto (use entero o decimal con hasta 5 decimales).',0Dh,0Ah,'$'
msgRangoInv   db 0Dh,0Ah,'Nota fuera de rango: debe estar entre 0 y 100 (100.00000 permitido).',0Dh,0Ah,'$'
msgOK         db 0Dh,0Ah,'Estudiante agregado! Pulse cualquier tecla para continuar.',0Dh,0Ah,'$'
msgLleno      db 0Dh,0Ah,'Capacidad maxima (15 estudiantes) alcanzada.',0Dh,0Ah,'$' 


; --- Opcion 3 (buscar por índice) ---
msgPedirIndice db 0Dh,0Ah,'Ingrese el indice del estudiante (1..15): ',0Dh,0Ah,'$'
msgSinDatos    db 0Dh,0Ah,'No hay estudiantes guardados.',0Dh,0Ah,'$'
msgNoNumerico  db 0Dh,0Ah,'Entrada invalida: debe ser un numero entero.',0Dh,0Ah,'$'
msgIndiceInv   db 0Dh,0Ah,'Indice fuera de rango.',0Dh,0Ah,'$'

; Buffer para leer un numero corto con AH=0Ah (max 3 dígitos)
bufferNum      db 3       ; capacidad maxima (3)
               db 0       ; longitud real
               db 3 dup(0)


; Buffer entrada AH=0Ah
bufferLinea   db 80
               db 0
               db 80 dup(0)

decCount      db 0       ; # decimales
fracNZ        db 0       ; hubo decimal
intVal        dw 0       ; parte entera acumulada (0..100)

; Almacenamiento de hasta 15 registros
studentsCount db 0
studentsBuf   db 15*80 dup(0)

opcion        db ?
