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
msgOK         db 0Dh,0Ah,'Estudiante agregado! Presione cualquier tecla para continuar.',0Dh,0Ah,'$'
msgLleno      db 0Dh,0Ah,'Capacidad maxima (15 estudiantes) alcanzada.',0Dh,0Ah,'$'


; --- Opcion 2 ---
msgMax db 0Dh,0Ah,'Nota maxima:',0Dh,0Ah,'$'
msgMin db 0Dh,0Ah,'Nota minima:',0Dh,0Ah,'$'
msgAprobados  db 13,10,'Cantidad de estudiantes aprobados: $'
msgReprobados  db 13,10,'Cantidad de estudiantes reprobados: $'
msgPorcAprob   db 13,10,'Porcentaje aprobados: $'
msgPorcReprob  db 13,10,'Porcentaje reprobados: $'


; --- Opcion 3 (buscar por ?ndice) ---
msgPedirIndice db 0Dh,0Ah,'Ingrese el indice del estudiante (1..15): ',0Dh,0Ah,'$'
msgSinDatos    db 0Dh,0Ah,'No hay estudiantes guardados.',0Dh,0Ah,'$'
msgNoNumerico  db 0Dh,0Ah,'Entrada invalida: debe ser un numero entero.',0Dh,0Ah,'$'
msgIndiceInv   db 0Dh,0Ah,'Indice fuera de rango.',0Dh,0Ah,'$'
msgNombreInv db 0Dh,0Ah,'Entrada invalida: debe ingresar Nombre y dos Apellidos antes de la nota.',0Dh,0Ah,'$'




; --- Opcion 4(Ordenar)
msgElegirOrden db 0Dh,0Ah,'Indique como desea ordenar las calificaciones:',0Dh,0Ah,'$'
msgElegirAsc   db 0Dh,0Ah,'1. Ascendente',0Dh,0Ah,'$'
msgElegirDes   db 0Dh,0Ah,'2. Descendente',0Dh,0Ah,'$'
msgOrdenAsc       db 0Dh,0Ah,'Calificaciones ordenadas (ascendente):',0Dh,0Ah,'$'
msgOrdenDesc       db 0Dh,0Ah,'Calificaciones ordenadas (descendente):',0Dh,0Ah,'$'
msgOrdenNoEnough  db 0Dh,0Ah,'Se requieren al menos 2 estudiantes para ordenar.',0Dh,0Ah,'$'
msgInvalido  db 0Dh,0Ah,'Presione la tecla 1 o 2',0Dh,0Ah,'$'
msgRegresarMenu  db 0Dh,0Ah, "Presione una tecla para continuar", 0Dh,0Ah, "$"
;Flag BubbleSort
ordenFlag db 0   ; 0 = Ascendente, 1 = Descendente
