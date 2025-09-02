org 100h

INCLUDE const.inc

start:
    call Cls
MainLoop:
    call MostrarMenu
    call LeerOpcion
    jmp  start   
    

; ---------- UI / Menú ----------
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

; ---------- Lógica de opciones ----------
LeerOpcion proc
    mov ah,1
    int 21h
    mov opcion, al

    cmp al, '1'
    je  IngresarCalificaciones 
    
    cmp al, '2'
    je  MostrarEstadisticas 
    
    cmp al, '3'
    je  BuscarPorIndice
    
    cmp al, '4'
    je  OrdenarCalificacionesMenu


    cmp al, '5'
    je  Salir

    mov dx, offset salto
    call PrintString
    mov dl, opcion
    mov ah,2
    int 21h
    mov dx, offset salto2
    call PrintString
    ret
LeerOpcion endp

; Utilidades y procs de UI (incluye PrintString/Cls/CRLF/PrintNumDec)
INCLUDE ui.inc

; Procs de estudiantes: IngresarCalificaciones, BuscarPorIndice…
INCLUDE students.inc

; Salir
Salir:
    mov dx, offset mensajeSalir
    call PrintString
    mov ax,4C00h
    int 21h

; Datos y mensajes (pueden ir al final sin problema en .COM)
INCLUDE messages.inc
INCLUDE data.inc
