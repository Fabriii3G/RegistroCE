; buffers
bufferNum      db 3,0,3 dup(0)
bufferLinea    db 80,0,80 dup(0)

tmpLo   dw 0
tmpHi   dw 0
tmpOfs  dw 0

; parse
decCount       db 0
fracNZ         db 0
intVal         dw 0
fracVal        dw 0

; almacenamiento
studentsCount  db 0
studentsBuf    db MAX_STUDENTS*REC_SIZE dup(0)
recOfs         dw MAX_STUDENTS dup(0)
notas32        dd MAX_STUDENTS dup(0)
notas16 dw 15 dup(?)   ; notas enteras en 0..100
msgNoNotas db 'No hay notas ingresadas.$'
msgProm db 'Promedio general: $'
sumLo dw 0
sumHi dw 0
promedio dw 0


; varios
opcion         db ?
