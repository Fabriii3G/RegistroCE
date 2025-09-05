 ;----------------------------------------------------
;Opcion 4. Ordenar calificaciones 
;  - Bubble sort en notas32[] (32-bit)
;  - Permuta en paralelo recOfs[]
;  - Imprime lista ordenada
;  - Referencia base: Ahmed  Ramadan,  Bubble  sort  on array  on Assembly Language Overflow, 
;               Stack overflow https://stackoverflow.com/questions/30427394/bubble-sort-on-array-on-assembly-language
;  - Generado parcialmente con ayuda de IA (ChatGPT).
;----------------------------------------------------
OrdenarCalificacionesMenu proc
    ; --- verificar si hay al menos 2 registros ---
    mov  al, [studentsCount]
    cmp  al, 2
    jae  ContinuarOrdenamiento

    ; si no hay suficientes, mostrar mensaje y regresar
    call Cls
    mov  dx, offset msgOrdenNoEnough
    call PrintString

    mov  ah, 7         ; esperar tecla antes de regresar
    int 21h
    call Cls
    ret

ContinuarOrdenamiento:
    call Cls
    mov  dx, offset msgElegirOrden
    call PrintString
    mov  dx, offset msgElegirAsc
    call PrintString
    mov  dx, offset msgElegirDes
    call PrintString

LeerOpcion4:
    mov ah, 1
    int 21h

    cmp al, '1'
    je OrdenarAsc
    cmp al, '2'
    je OrdenarDesc

    ; opcion invalida
    mov dx, offset msgInvalido
    call PrintString
    jmp LeerOpcion4   ; vuelve a pedir

OrdenarAsc:
    mov [ordenFlag], 0
    call OrdenarCalificacionesBubble
    ret

OrdenarDesc:
    mov [ordenFlag], 1
    call OrdenarCalificacionesBubble
    ret
OrdenarCalificacionesMenu endp


;----------------------------------------------------
; Ordenamiento Bubble con flag asc/desc
;----------------------------------------------------
OrdenarCalificacionesBubble proc
    call Cls

    mov  cl, [studentsCount]
    dec  cl              ; CL = n-1 (pasadas del bubble)

OuterLoop:
    mov  ch, cl
    xor  si, si          ; j = 0

InnerLoop:
    ; apuntar a notas32[j]
    mov  ax, si
    shl  ax, 2
    mov  di, offset notas32
    add  di, ax

    mov  ax, [di]
    mov  dx, [di+2]

    mov  bx, [di+4]
    mov  bp, [di+6]

    ; --- Comparacion segun flag ---
    mov  al, [ordenFlag]
    cmp  al, 0
    je   AscendenteCmp

DescendenteCmp:
    ; DESC: swap si nota[j] < nota[j+1]
    cmp  dx, bp
    ja   NoSwap
    jb   DoSwap
    cmp  ax, bx
    jae  NoSwap
    jmp DoSwap

AscendenteCmp:
    ; ASC: swap si nota[j] > nota[j+1]
    cmp  dx, bp
    jb   NoSwap
    ja   DoSwap
    cmp  ax, bx
    jbe  NoSwap
    jmp DoSwap

; Intercambio de notas
DoSwap:
    ; Intercambio en notas32[j] <-> notas32[j+1]
    xchg ax, bx
    xchg dx, bp
    mov [di], ax
    mov [di+2], dx
    mov [di+4], bx
    mov [di+6], bp

    ; Intercambio recOfs[j] <-> recOfs[j+1]
    mov ax, si
    shl ax, 1
    mov di, offset recOfs
    add di, ax
    mov ax, [di]
    xchg ax, [di+2]
    mov [di], ax


NoSwap:
    inc  si
    dec  ch
    jnz  InnerLoop

    dec  cl
    jnz  OuterLoop

    ; ----- Imprimir resultado -----
    mov  al, [ordenFlag]
    cmp  al, 0
    je   AscPrint
DescPrint:
    mov  dx, offset msgOrdenDesc
    jmp ContinuePrint
AscPrint:
    mov  dx, offset msgOrdenAsc
ContinuePrint:
    call PrintString

    xor  bx, bx
PrintLoop:
    mov  al, [studentsCount]
    cmp  bl, al
    jae  FinPrint

    mov  ax, bx
    shl  ax, 1
    mov  si, offset recOfs
    add  si, ax
    mov  di, [si]

    mov  dx, di
    call PrintString

    inc  bl
    jmp PrintLoop

FinPrint:
    mov  dx, offset msgRegresarMenu
    call PrintString

    mov ah, 7         ; esperar input
    int 21h

    call Cls
    ret
OrdenarCalificacionesBubble endp