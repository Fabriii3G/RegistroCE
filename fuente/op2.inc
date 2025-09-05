;----------------------------------------------------
; Opcion 2: Mostrar estadísticas
;   - Nota más alta y más baja
;   - Cantidad de estudiantes aprobados (nota >= 70)
;   - Imprime la línea completa del estudiante
;   - Generado parcialmente con ayuda de IA (ChatGPT).
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
    mov  dx, [si+2]         
    mov  word ptr maxLo, ax
    mov  word ptr maxHi, dx
    mov  word ptr minLo, ax
    mov  word ptr minHi, dx
    mov  word ptr idxMax, 0
    mov  word ptr idxMin, 0

    ; Inicializar contador de aprobados/reprobados
    mov  word ptr reprobados, 0
    mov  word ptr aprobados, 0

    ; Revisar primera nota
    mov  ax, [si]
    mov  dx, [si+2]
    call CheckAprobado

    ; Contador: CX 
    xor  cx, cx             
    mov  cl, [studentsCount]
    dec  cx
    jz   UnicoEstudiante    ; si solo 1 estudiante, saltar el bucle
    mov  bx, 1              ; j = 1

LoopStats:
    ; nota[j] -> DX:AX
    mov  ax, bx
    shl  ax, 2              
    mov  si, offset notas32
    add  si, ax
    mov  ax, [si]           ; parte baja
    mov  dx, [si+2]         ; parte alta (notaEscalada)

    ; comparar con max
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
    ; comparar con min
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
    ; Promedio 
    call CRLF
    mov  dx, offset msgProm
    call PrintString
    call CalcAndPrintPromedio5

    ; Línea en blanco
    call CRLF

    ; Imprimir resultados 
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

    ; Imprimir cantidad de aprobados
    mov  dx, offset msgAprobados
    call PrintString
    mov  ax, [aprobados]
    call PrintNumber

    ; porcentaje
    mov  ah, 02h
    mov  dl, ' '
    int  21h

    mov  dx, offset msgPorcAprob
    add  dx, 2                 
    call PrintString
    mov  ax, [aprobados]      ; valor
    call CalcPorcentaje      
    call PrintNumber

    ; imprimir '%' al lado del número
    mov  ah, 02h
    mov  dl, '%'
    int  21h

    ; Línea en blanco
    call CRLF

    ; Imprimir cantidad de reprobados 
    mov  dx, offset msgReprobados
    call PrintString
    mov  ax, [reprobados]
    call PrintNumber

    mov  ah, 02h
    mov  dl, ' '
    int  21h

    mov  dx, offset msgPorcReprob
    add  dx, 2                
    call PrintString
    mov  ax, [reprobados]     ; valor
    call CalcPorcentaje       ; % reprobados
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
; Subrutina: Verifica si >= 70 escalado
; Entrada: DX:AX = notaEscalada (32-bit)
; Salida: incrementa "aprobados" o "reprobados"
;----------------------------------------------------
CheckAprobado proc
    cmp dx, 006Ah
    ja  EsAprobado          ;  seguro >= 7,000,000
    jb  NoAprobado          ;  seguro <  7,000,000
    cmp ax, 0CFC0h          ;  comparar parte baja
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
    mov bh, 0                 
    mov cx, 100
    xor dx, dx               
    mul cx                   
    div bx                    ; AX = (valor * 100) / studentsCount
    ret
PorcentajeCero:
    xor ax, ax                ; AX = 0
    ret
CalcPorcentaje endp


;----------------------------------------------------
; CalcAndPrintPromedio5
; Suma notas32[] (32-bit, escaladas ×100000),
; divide entre studentsCount y imprime: ENTERO '.' 5 decimales
; Sin overflow al formar los 5 decimales.
;----------------------------------------------------
CalcAndPrintPromedio5 proc
    ; Si no hay estudiantes, imprime 0.00000
    mov  al, [studentsCount]
    cmp  al, 0
    jne  CAP_Continuar
    mov  ax, 0
    call PrintNumber
    mov  ah, 02h
    mov  dl, '.'
    int  21h
    mov  dl, '0'            ; primer decimal
    mov  ah, 02h
    int  21h
    xor  ax, ax             ; otros 4 decimales 
    call Print4Digits
    ret

CAP_Continuar:
    ; --- suma 32-bit ---
    mov  word ptr [sumLo], 0
    mov  word ptr [sumHi], 0
    xor  bx, bx
    xor  cx, cx                ; *** FIX: limpia todo CX
    mov  cl, [studentsCount]   ; CL = n (0..15)
    jcxz CAP_Dividir

CAP_SumLoop:
    mov  ax, bx
    shl  ax, 2                 ; offset 
    mov  si, offset notas32
    add  si, ax
    mov  ax, [si]              ; low
    add  [sumLo], ax
    mov  ax, [si+2]            ; high
    adc  [sumHi], ax
    inc  bx
    loop CAP_SumLoop

    ; --- REDONDEO AL 5to DECIMAL ---
    mov  bl, [studentsCount]
    xor  bh, bh           ; BX = n
    mov  ax, bx
    shr  ax, 1            ; AX = floor(n/2)

    ; sumHi:sumLo += AX
    add  [sumLo], ax
    adc  [sumHi], 0


CAP_Dividir:
    mov  bl,[studentsCount]
    xor  bh,bh
    mov  bx,bx           ; BX = n

    ; dividir sumHi:sumLo / BX
    mov  ax,[sumHi]
    mov  dx,0
    div  bx              ; AX = cociente alto
    mov  si,ax           ; tmpHi = cociente alto

    mov  ax,[sumLo]
    ; DX = resto de la división
    div  bx              ; AX = cociente bajo
    mov  [tmpLo],ax
    mov  [tmpHi],si


    ; separar entero y 5 decimales 
    ; N = [tmpHi]:[tmpLo]  (promedio escalado ×100000)

    ; dividir por 10000 -> q10 (×10) y rem10 (0..9999)
    mov  dx, [tmpHi]
    mov  ax, [tmpLo]
    mov  bx, 10000
    div  bx                    
    mov  si, dx              

    ; dividir q10 por 10 -> entero y remA (0..9)
    xor  dx, dx
    mov  bx, 10
    div  bx                    

    ; imprimir entero
    push dx                    
    call PrintNumber

    ; imprimir '.'
    mov  ah, 02h
    mov  dl, '.'
    int  21h

    ; imprimir primer decimal 
    pop  dx
    add  dl, '0'
    mov  ah, 02h
    int  21h

    ; imprimir los otros 4 decimales con ceros a la izquierda
    mov  ax, si
    call Print4Digits
    ret
CalcAndPrintPromedio5 endp

;----------------------------------------------------
; Print4Digits
; Entrada: AX en 0..9999
; Salida : imprime los ultimos 4 bits (toma en cuenta ceros)
;----------------------------------------------------
Print4Digits proc
    push ax
    push bx
    push cx
    push dx

    mov  bx, 10
    mov  cx, 4

P4_Loop:
    xor  dx, dx
    div  bx
    push dx
    loop P4_Loop

    mov  cx, 4
P4_Out:
    pop  dx
    add  dl, '0'
    mov  ah, 02h
    int  21h
    loop P4_Out

    pop  dx
    pop  cx
    pop  bx
    pop  ax
    ret
Print4Digits endp


; ---- Variables auxiliares ----
maxLo      dw 0
maxHi      dw 0
minLo      dw 0
minHi      dw 0
idxMax     dw 0
idxMin     dw 0
reprobados dw 0
aprobados  dw 0