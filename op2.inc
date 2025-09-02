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
    ; ---- Promedio general ----
    call CRLF
    mov  dx, offset msgProm     ; "Promedio general: "
    call PrintString
    call MostrarPromedioGeneral ; imprime XX.YYYYY

    ; --- Línea en blanco después del promedio ---
    call CRLF

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

    ; poner un espacio y luego la etiqueta de porcentaje (saltando CRLF inicial)
    mov  ah, 02h
    mov  dl, ' '
    int  21h

    mov  dx, offset msgPorcAprob
    add  dx, 2                 ; salto los 0Dh,0Ah iniciales para que quede en la misma línea
    call PrintString
    mov  ax, [aprobados]      ; valor
    call CalcPorcentaje       ; AX = % aprobados
    call PrintNumber
    ; imprimir '%' al lado del número
    mov  ah, 02h
    mov  dl, '%'
    int  21h

    ; ---- Línea en blanco separadora ----
    call CRLF

    ; ---- Imprimir cantidad de reprobados ----
    mov  dx, offset msgReprobados
    call PrintString
    mov  ax, [reprobados]
    call PrintNumber

    ; poner un espacio y luego la etiqueta de porcentaje (saltando CRLF inicial)
    mov  ah, 02h
    mov  dl, ' '
    int  21h

    mov  dx, offset msgPorcReprob
    add  dx, 2                 ; salto los 0Dh,0Ah iniciales
    call PrintString
    mov  ax, [reprobados]     ; valor
    call CalcPorcentaje       ; AX = % reprobados
    call PrintNumber
    ; imprimir '%' al lado del número
    mov  ah, 02h
    mov  dl, '%'
    int  21h

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
    mov bl, [studentsCount]   ; BL = total estudiantes
    cmp bl, 0
    je PorcentajeCero         ; Si no hay estudiantes, porcentaje = 0
    mov bh, 0                 ; BX = studentsCount (16 bits)
    mov cx, 100
    xor dx, dx                ; Limpia DX antes de mul
    mul cx                    ; DX:AX = valor * 100
    div bx                    ; AX = (valor * 100) / studentsCount
    ret
PorcentajeCero:
    xor ax, ax                ; AX = 0
    ret
CalcPorcentaje endp


;----------------------------------------------------
; MostrarPromedioGeneral
; - Suma notas32[] (32-bit, escaladas por 100000)
; - Divide entre studentsCount para obtener promedio escalado
; - Convierte y muestra con 2 decimales: XX.XX (redondeado)
;   (sin perder precisión del escalado)
;----------------------------------------------------
MostrarPromedioGeneral proc
    ; Si no hay estudiantes, muestra "0.00" por seguridad
    mov  al, [studentsCount]
    cmp  al, 0
    jne  MP_Continuar
    mov  ax, 0
    call PrintNumber
    mov  ah, 02h
    mov  dl, '.'
    int  21h
    mov  ax, 0
    call Print2Digits
    ret

MP_Continuar:
    ; sum = 0 (32-bit)
    mov  word ptr [sumLo], 0
    mov  word ptr [sumHi], 0

    ; loop j=0..studentsCount-1
    xor  bx, bx                      ; j = 0
    mov  cl, [studentsCount]
    xor  ch, ch
    jcxz MP_ListoSum

MP_LoopSum:
    mov  ax, bx
    shl  ax, 2                       ; j*4 (dd)
    mov  si, offset notas32
    add  si, ax

    ; sum += notas32[j]  (sumLo/sumHi += [si]:[si+2])
    mov  ax, [si]                    ; low
    add  [sumLo], ax
    mov  ax, [si+2]                  ; high
    adc  [sumHi], ax

    inc  bx
    loop MP_LoopSum

MP_ListoSum:
    ; avgScaled (32-bit) = sum (32-bit) / count (16-bit)
    mov  bl, [studentsCount]         ; divisor (<=15)
    xor  bh, bh
    mov  ax, [sumHi]                 ; paso 1: dividir la parte alta
    xor  dx, dx
    div  bx                          ; AX = q1 (alto), DX = r1
    mov  [tmpHi], ax                 ; guardar q1
    mov  ax, [sumLo]                 ; paso 2: dividir r1:lo
    div  bx                          ; AX = q0 (bajo), DX = r0
    mov  [tmpLo], ax                 ; avgScaled = tmpHi:tmpLo (32-bit)

    ; Queremos mostrar con 2 decimales.
    ; Truco: (avgScaled * 100)/100000  == avgScaled/1000
    ; Redondeo: sumamos 500 antes de dividir entre 1000.
    mov  ax, [tmpLo]
    add  ax, 500                     ; +500 para redondeo en /1000
    mov  [tmpLo], ax
    mov  ax, [tmpHi]
    adc  ax, 0
    mov  [tmpHi], ax

    ; q = (tmpHi:tmpLo) / 1000   -> q = valor * 100 (XXYY, donde XX=entero, YY=decimales)
    mov  ax, [tmpHi]
    xor  dx, dx
    mov  bx, 1000
    div  bx                          ; AX=q1', DX=r1'
    mov  [tmpHi], ax
    mov  ax, [tmpLo]
    div  bx                          ; AX=q0', DX=r0'
    mov  [tmpLo], ax                 ; q (32-bit) = tmpHi:tmpLo

    ; Como el promedio está entre 0..100, q <= 10000 -> la parte alta es 0
    ; entero = q / 100 ; dec = q % 100
    mov  ax, [tmpLo]
    xor  dx, dx
    mov  bx, 100
    div  bx                          ; AX = entero, DX = dec (0..99)

    ; imprimir entero
    push dx                          ; guardar dec
    call PrintNumber

    ; imprimir '.'
    mov  ah, 02h
    mov  dl, '.'
    int  21h

    ; recuperar decimales en AX y mostrar 2 dígitos
    pop  ax
    call Print2Digits

    ret
MostrarPromedioGeneral endp

;----------------------------------------------------
; Print2Digits
; Entrada: AX en 0..99
; Salida : imprime dos dígitos (con 0 a la izquierda si < 10)
;----------------------------------------------------
Print2Digits proc
    push ax
    push bx
    ; dividir AX entre 10 -> AX = decenas, DX = unidades
    mov  bx, 10
    xor  dx, dx
    div  bx             ; AX = tens (0..9), DX = units (0..9)

    push dx             ; guardar unidades (DX) en la pila, porque INT 21h puede clobber DX

    ; imprimir decenas (AL)
    mov  dl, al
    add  dl, '0'
    mov  ah, 02h
    int  21h

    ; recuperar unidades en DX y convertir a carácter
    pop  dx              ; DX = unidades
    add  dl, '0'
    mov  ah, 02h
    int  21h

    pop  bx
    pop  ax
    ret
Print2Digits endp



; ---- Variables auxiliares ----
maxLo      dw 0
maxHi      dw 0
minLo      dw 0
minHi      dw 0
idxMax     dw 0
idxMin     dw 0
reprobados dw 0
aprobados  dw 0