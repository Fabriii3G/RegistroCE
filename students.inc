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


;----------------------------------------------------
; Opcion 2: Mostrar estadísticas
;   - Nota más alta y más baja
;   - Cantidad de estudiantes aprobados (nota >= 70)
;   - Imprime la línea completa del estudiante
;----------------------------------------------------
MostrarEstadisticas proc
    call Cls

    ; ¿Hay estudiantes?
    mov  al, [studentsCount]
    cmp  al, 0
    jne  TieneDatosStats
    mov  dx, offset msgSinDatos
    call PrintString
    mov ah,1
    int  21h
    ret

TieneDatosStats:
    ; Inicializar min y max con la primera nota (notas32[0])
    xor  bx, bx
    mov  si, offset notas32
    mov  ax, [si]           ; low word
    mov  dx, [si+2]         ; high word (notaEscalada)
    mov  word ptr maxLo, ax
    mov  word ptr maxHi, dx
    mov  word ptr minLo, ax
    mov  word ptr minHi, dx
    mov  word ptr idxMax, 0
    mov  word ptr idxMin, 0

    ; Inicializar contador de aprobados/reprobados
    mov  word ptr reprobados, 0
    mov  word ptr aprobados, 0

    ; ---- Revisar primera nota ----
    mov  ax, [si]
    mov  dx, [si+2]
    call CheckAprobado

    ; ===== Preparar contador: CX = studentsCount - 1 =====
    xor  cx, cx             ; limpiar CH:CL
    mov  cl, [studentsCount]
    dec  cx
    jz   UnicoEstudiante    ; si solo 1 estudiante, saltar el bucle
    mov  bx, 1              ; j = 1

LoopStats:
    ; nota[j] -> DX:AX
    mov  ax, bx
    shl  ax, 2              ; j*4
    mov  si, offset notas32
    add  si, ax
    mov  ax, [si]           ; parte baja
    mov  dx, [si+2]         ; parte alta (notaEscalada)

    ; comparar con max (DX:AX vs maxHi:maxLo)
    cmp  dx, [maxHi]
    ja   NewMax
    jb   CheckMin
    cmp  ax, [maxLo]
    jbe  CheckMin
NewMax:
    mov  [maxLo], ax
    mov  [maxHi], dx
    mov  [idxMax], bx

CheckMin:
    ; comparar con min (DX:AX vs minHi:minLo)
    cmp  dx, [minHi]
    jb   NewMin
    ja   NextJ
    cmp  ax, [minLo]
    jae  NextJ
NewMin:
    mov  [minLo], ax
    mov  [minHi], dx
    mov  [idxMin], bx

NextJ:
    ; verificar si es aprobado (notaEscalada)
    call CheckAprobado

    inc  bx
    loop LoopStats

UnicoEstudiante:
    ; ---- Imprimir resultados ----
    mov  dx, offset msgMax
    call PrintString
    mov  ax, [idxMax]
    shl  ax, 1
    mov  si, offset recOfs
    add  si, ax
    mov  di, [si]
    mov  dx, di
    call PrintString

    mov  dx, offset msgMin
    call PrintString
    mov  ax, [idxMin]
    shl  ax, 1
    mov  si, offset recOfs
    add  si, ax
    mov  di, [si]
    mov  dx, di
    call PrintString

    ; ---- Imprimir cantidad de aprobados ----
    mov  dx, offset msgAprobados
    call PrintString
    mov  ax, [aprobados]
    call PrintNumber

    ; ---- Porcentaje aprobados ----
    mov  ax, [aprobados]
    call CalcPorcentaje
    mov  dx, offset msgPorcAprob
    call PrintString
    call PrintNumber

    ; ---- Cantidad de reprobados ----
    mov  dx, offset msgReprobados
    call PrintString
    mov  ax, [reprobados]
    call PrintNumber

    ; ---- Porcentaje reprobados ----
    mov  ax, [reprobados]
    call CalcPorcentaje
    mov  dx, offset msgPorcReprob
    call PrintString
    call PrintNumber

    ; Pausa
    mov  ah,1
    int  21h
    ret
MostrarEstadisticas endp

;----------------------------------------------------
; Subrutina: Verifica si DX:AX >= 70 escalado
; Entrada: DX:AX = notaEscalada (32-bit)
; Salida: incrementa "aprobados" o "reprobados"
;----------------------------------------------------
;----------------------------------------------------
CheckAprobado proc
    cmp dx, 006Ah
    ja  EsAprobado          ; DX > 0x006A  → seguro >= 7,000,000
    jb  NoAprobado          ; DX < 0x006A  → seguro <  7,000,000
    cmp ax, 0CFC0h          ; DX = 0x006A  → comparar parte baja
    jae EsAprobado
    jmp NoAprobado

EsAprobado:
    inc word ptr aprobados
    ret

NoAprobado:
    inc word ptr reprobados
    ret
CheckAprobado endp

;----------------------------------------------------
; Calcula porcentaje = (valor * 100) / studentsCount
; Entrada: AX = valor
; Usa:     studentsCount (db)
; Salida:  AX = porcentaje (0–100)
;----------------------------------------------------
CalcPorcentaje proc
    mov bl, [studentsCount]  ; BL = total (8 bits)
    mov bh, 0                ; Extender a 16 bits → BX = 0..BL
    mov cx, 100
    mul cx                   ; DX:AX = valor * 100
    div bx                   ; AX = (valor*100)/total
    ret
CalcPorcentaje endp


; ---- Variables auxiliares ----
maxLo      dw 0
maxHi      dw 0
minLo      dw 0
minHi      dw 0
idxMax     dw 0
idxMin     dw 0
reprobados dw 0
aprobados  dw 0



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


;----------------------------------------------------
; Opcion 4: Ordenar calificaciones ascendente
;  - Bubble sort en notas32[] (32-bit)
;  - Permuta en paralelo recOfs[]
;  - Imprime lista ordenada
;  - NO usa CX dentro del inner loop (evita romper CH)
;----------------------------------------------------
OrdenarCalificacionesAsc proc
    call Cls

    mov  al, [studentsCount]
    cmp  al, 2
    jae  TieneDatos
    mov  dx, offset msgOrdenNoEnough
    call PrintString
    mov  ah,1
    int  21h
    ret

TieneDatos:
    mov  cl, [studentsCount]   ; CL = n
    dec  cl                    ; CL = n-1 (pasadas del bubble)

OuterLoop:
    mov  ch, cl                ; CH = cantidad de comparaciones en esta pasada
    xor  si, si                ; j = 0

InnerLoop:
    ; ptr = &notas32[j]
    mov  ax, si
    shl  ax, 2                 ; j*4
    mov  di, offset notas32
    add  di, ax

    ; nota[j]  -> DX:AX  (lo=ax, hi=dx)
    mov  ax, [di]
    mov  dx, [di+2]

    ; nota[j+1]-> BP:BX  (lo=bx, hi=bp)  (NO usar CX)
    mov  bx, [di+4]
    mov  bp, [di+6]

    ; comparar DX:AX vs BP:BX  (j ? j+1)
    cmp  dx, bp
    jb   NoSwap                 ; j < j+1  (ok asc)
    ja   HacerSwap              ; j > j+1  (swap)
    cmp  ax, bx
    jbe  NoSwap                 ; j <= j+1

HacerSwap:
    ; --- swap notas32[j] <-> notas32[j+1] ---
    xchg ax, bx                 ; lo
    xchg dx, bp                 ; hi
    mov  [di],   ax
    mov  [di+2], dx
    mov  [di+4], bx
    mov  [di+6], bp

    ; --- swap recOfs[j] <-> recOfs[j+1] ---
    mov  ax, si
    shl  ax, 1                  ; j*2
    mov  di, offset recOfs
    add  di, ax
    mov  ax, [di]               ; recOfs[j]
    xchg ax, [di+2]             ; con recOfs[j+1]
    mov  [di], ax

NoSwap:
    inc  si
    dec  ch
    jnz  InnerLoop

    dec  cl
    jnz  OuterLoop

    ; ----- imprimir ya ordenado -----
    mov  dx, offset msgOrdenAsc
    call PrintString

    xor  bx, bx
PrintLoop:
    mov  al, [studentsCount]
    cmp  bl, al
    jae  FinPrint

    ; DI = studentsBuf + recOfs[bx]
    mov  ax, bx
    shl  ax, 1
    mov  si, offset recOfs
    add  si, ax
    mov  di, [si]

    mov  dx, di
    call PrintString

    inc  bl
    jmp  PrintLoop

FinPrint:
    mov  ah,1
    int  21h
    call Cls
    ret
OrdenarCalificacionesAsc endp