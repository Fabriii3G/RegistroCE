; buffers
bufferNum      db 3,0,3 dup(0)
bufferLinea    db 100,0,100 dup(0)


tmpOfs  dw 0
intPartLo dw 0
intPartHi dw 0
sum0 dw 0   ; bits 0..15
sum1 dw 0   ; bits 16..31
sum2 dw 0   ; bits 32..47
sum3 dw 0   ; bits 48..63


; parse
decCount       db 0
fracNZ         db 0
intVal         dw 0
fracVal        dw 0

; --- 32-bit fraction (DWORD) ---
frac    dd 0          ; 32 bits: low word en frac, high word en frac+2
fracLo  equ frac      ; alias a la mitad baja (dirección frac)
fracHi  equ frac+2    ; alias a la mitad alta (dirección frac+2)


; almacenamiento
studentsCount  db 0
studentsBuf db 2048 dup(0)    ; 15 * 80 = 1200
recOfs         dw MAX_STUDENTS dup(0)
notas32        dd MAX_STUDENTS dup(0)
notas16 dw 15 dup(?)   ; notas enteras en 0..100
msgNoNotas db 'No hay notas ingresadas.$'

; auxiliares para promedio
tmpLo   dw 0
tmpHi   dw 0
sumLo   dw 0   ; bits 0..15
sumHi   dw 0   ; bits 16..31


; En la sección de datos
parteEntera    dw 0
residuoHi      dw 0
residuoLo      dw 0
sumHi2         dw 0  ; para suma de 64 bits
sumHi4         dw 0  ; para suma de 64 bits




; varios
opcion         db ?
