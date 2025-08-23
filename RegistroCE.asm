org 100h    ; 

start:
    call Cls             ; limpiar pantalla
    call MostrarMenu     ; desplegar menu principal
    call LeerOpcion      ; leer opcion del usuario
    jmp start            ; volver al menu (bucle principal)


; Subrutina: MostrarMenu
MostrarMenu proc
    ; Titulo
    mov dx, offset titulo
    call PrintString

    ; Opciones
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


; Subrutina: LeerOpcion
LeerOpcion proc
    mov ah, 1       ; leer caracter (espera tecla)
    int 21h
    mov opcion, al  ; guardar la tecla presionada

    ; mostrar salto de linea
    mov dx, offset salto
    call PrintString

    ; Solo se muestra lo que elige el usuario
    mov dl, opcion
    mov ah, 2
    int 21h

    mov dx, offset salto2
    call PrintString
    ret
LeerOpcion endp


; Subrutina: PrintString
PrintString proc
    mov ah, 9
    int 21h
    ret
PrintString endp



; Subrutina: Cls (limpiar pantalla)
Cls proc
    mov ax, 0600h
    mov bh, 07
    mov cx, 0000h
    mov dx, 184fh
    int 10h
    mov ah, 02h
    mov bh, 0
    mov dx, 0000h
    int 10h
    ret
Cls endp


; Datos
titulo  db 0Dh,0Ah, '|RegistroCE|',0Dh,0Ah,'$'
op1     db '1. Ingresar calificaciones',0Dh,0Ah,'$'
op2     db '2. Mostrar estadisticas',0Dh,0Ah,'$'
op3     db '3. Buscar estudiante',0Dh,0Ah,'$'
op4     db '4. Ordenar calificaciones',0Dh,0Ah,'$'
op5     db '5. Salir',0Dh,0Ah,'$'
elegir  db 0Dh,0Ah,'Seleccione una opcion: $'
salto   db 0Dh,0Ah,'Ha elegido: $'
salto2  db 0Dh,0Ah,'Presione una tecla para continuar...',0Dh,0Ah,'$'
opcion  db ?


; Fin del programa
ret
