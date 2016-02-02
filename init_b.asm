.text
#        ENTRY                  /*  mark the first instruction to call */
.global patron_volteo_arm
.global patron_volteo_thumb
/*indicates that we are using the ARM instruction set */
#------standard initial code
# --- Setup interrupt / exception vectors

/* In this version we do not use the following handlers */
#######################################################################################################
#-----------Undefined_Handler:
#      B       Undefined_Handler
#----------SWI_Handler:
#      B       SWI_Handler
#----------Prefetch_Handler:
#      B       Prefetch_Handler
#----------Abort_Handler:
#      B       Abort_Handler
#         NOP      /* Reserved vector */
#----------IRQ_Handler:
#      B       IRQ_Handler
#----------FIQ_Handler:
#      B       FIQ_Handler
#######################################################################################################
# Reset Handler:
# the processor starts executing this code after system reset
#######################################################################################################

#######################################################################################################
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FICHA VALIDA
// Devuelve el contenido de la posicion indicada por la fila y columna actual.
// Ademas informa si la posicion es valida y contiene alguna ficha. Esto lo hace por referencia (en *posicion_valida).
// Si devuelve un 0 no es valida o esta vacia.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
.arm
#######################################################################################################
ficha_valida_arm:
	// r0 = fila actual
	// r1 = columna actual
	// r2 = @tablero
	// r3 = @posicion_valida

	//char ficha = tablero[fila_actual][columna_actual];
	add r2,r2,r0,LSL #3
	ldrb r2,[r2,r1]		// r2 = ficha

	//	if ((fila_actual < 0) ||
	//	   (columna_actual < 0) ||
	//	   (fila_actual > 7) ||
	//	   (columna_actual > 7) ||
	//	   (ficha == 0)) {
	//			*posicion_valida = 0;
	//	}
	//	else {
	//			*posicion_valida = 1;
	//	}

	cmp r0,#0
	cmpge r1,#0
	blt cero
	cmp r0,#7
	cmple r1,#7
	bgt cero
	cmp r2,#0
	movne r1,#1
	bne uno
cero:
	mov r1,#0
uno:
	str r1,[r3]	// @posicion_valida <- posicion_valida
	mov r0,r2	// return ficha
	mov pc,lr

##############################################################################################

.thumb
ficha_valida_thumb:
	// r0 = fila actual
	// r1 = columna actual
	// r2 = @tablero
	// r3 = @posicion_valida

	//char ficha = tablero[fila_actual][columna_actual];
	push {r4}
	LSL r4,r0,#3
	add r2,r2,r4
	ldrb r2,[r2,r1]		// r2 = ficha

	//	if ((fila_actual < 0) ||
	//	   (columna_actual < 0) ||
	//	   (fila_actual > 7) ||
	//	   (columna_actual > 7) ||
	//	   (ficha == 0)) {
	//			*posicion_valida = 0;
	//	}
	//	else {
	//			*posicion_valida = 1;
	//	}

	cmp r0,#0
	blt cero_thumb
	cmp r1,#0
	blt cero_thumb
	cmp r0,#7
	bgt cero_thumb
	cmp r1,#7
	bgt cero_thumb
	cmp r2,#0
	beq cero_thumb
	mov r1,#1
	b uno_thumb
cero_thumb:
	mov r1,#0
uno_thumb:
	str r1,[r3]	// @posicion_valida <- posicion_valida
	mov r0,r2	// return ficha
	pop {r4}
	mov pc,lr

#######################################################################################################
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PATRON VOLTEO
// Comprueba si hay que actualizar una determinada direccion, para ello busca el patron de volteo:
// n fichas del rival seguidas de una ficha del jugador actual. SF y SC son las cantidades a sumar para movernos en la direccion
// que toque. Color indica el color de la pieza que se acaba de colocar. La funcion devuelve el numero de fichas que habra que voltear.
// f y c son la fila y columna a analizar.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#######################################################################################################
.arm
patron_volteo_arm:
	// r0 = f
	// r1 = c
	// r2 = SF
	// r3 = SC
	// color y tablero se pasan por memoria
	// @(fp+4) = color
	// @(fp+8) = @tablero

	//Se prepara el marco de la pila y se salvan los registros
	mov r12,sp
	stmdb sp!, {r4-r12,lr,pc}
	sub r11,r12,#4
	sub sp,sp,#4 		// espacio variables locales

	mov r4,#0			// r4 = iter
	add r0,r0,r2		// r0 = FA = f + SF
	add r1,r1,r3		// r1 = CA = c + SC
	and r0,r0,#0xFF		// Tratamiento de signo de FA
	and r1,r1,#0xFF		// Tratamiento de signo de CA
	mov r5,r2			// r5 = SF
	mov r6,r3			// r6 = SC
	sub r3,fp,#44		// r3 = @posicion_valida

	// Salvamos variables locales
	mov r9,r0		// r9 = FA
	mov r10,r1		// r10 = CA

	//Se cargan los parametros que se han pasado por memoria
	ldrb r7,[r11,#4]	//r7 = color
	ldr r2,[r11,#8]		//r2 = @tablero

	// ficha_valida_arm(FA, CA, tablero, &posicion_valida)
	bl ficha_valida_arm
	// r0 = casilla

	//while ((posicion_valida == 1) && (casilla != color))
in:
	//cargamos las variables necesarias
	sub r3,fp,#44			// r3 = @posicion_valida
	ldr r8,[r3]				// r8 = posicion_valida
	//comprobacion de la condicion del bucle
	cmp r8,#1 				// (posicion_valida == 1)
	bne out
	cmp r0,r7 				// (casilla != color)
	beq out
	add r9,r9,r5			// FA = FA + SF
	add r10,r10,r6			// CA = CA + SC
	and r9,r9,#0xFF			// Tratamiento de signo de FA
	and r10,r10,#0xFF		// Tratamiento de signo de CA

	add r4,r4,#1			// iter++

	//Parametros para ficha_valida
	mov r0,r9
	mov r1,r10
	ldr r2,[r11,#8]			// r2 = @tablero
	//ficha_valida_arm(FA, CA, tablero, &posicion_valida)
	bl ficha_valida_arm
	b in
out:
	//if ((posicion_valida == 1) && (casilla == color))
	cmp r8,#1
	cmpeq r0,r7
	moveq r0,r4		// return = iter
	movne r0,#0		// return = 0

	//restauramos los valores de la pila
	ldmdb fp, {r4-r11,r13,pc}

##############################################################################################

patron_volteo_thumb:

	push {r0}
	adr r0, patron_inicio_thumb + 1
	bx r0		// Cambia de conjunto de instrucciones

	//Se prepara el marco de la pila y se salvan los registros

.thumb
patron_inicio_thumb:
	// r0 = f
	// r1 = c
	// r2 = SF
	// r3 = SC
	// color y tablero se pasan por memoria
	// @(fp+4) = color
	// @(fp+8) = @tablero

	pop {r0}
	push {r4-r7,lr}

	// r7 = fp
	add r7,sp,#16
	sub sp,sp,#8 			// espacio variables locales

	mov r4,#0xFF
	add r0,r0,r2			// r0 = FA = f + SF
	and r0,r0,r4
	add r1,r1,r3			// r1 = CA = c + SC
	and r1,r1,r4

	// Salvamos variables locales CA y FA
	mov r4,r7
	sub r4,#18
	strb r1,[r4]			// @fp-18 = CA
	strb r0,[r4,#1]			// @fp-17 = FA

	mov r4,#0				// r4 = iter
	mov r5,r2				// r5 = SF
	mov r6,r3				// r6 = SC
	mov r3,r7
	sub r3,#24				// r3 = @posicion_valida

	//Se cargan los parametros que se han pasado por memoria
	ldr r2,[r7,#8]			//r2 = @tablero

	// ficha_valida_thumb(FA, CA, tablero, &posicion_valida)
	bl ficha_valida_thumb
	// r0 = casilla

	//while ((posicion_valida == 1) && (casilla != color))
in_thumb:
	//cargamos las variables necesarias
	mov r3,r7
	sub r3,#24				// r3 = @posicion_valida
	ldr r2,[r3]				// r2 = posicion_valida
	ldrb r1,[r7,#4]			// r1 = color
	//comprobacion de la condicion del bucle
	cmp r2,#1 				// (posicion_valida == 1)
	bne out_thumb
	cmp r0,r1				// (casilla != color)
	beq out_thumb
	mov r2,r7
	sub r2,#18
	ldrb r1,[r2]			// r1 = CA @(fp-18)
	add r1,r1,r6			// CA = CA + SC
	// Salvamos variable local 'CA'
	mov r0,#0xFF
	and r1,r1,r0
	strb r1,[r2]			// @(fp-18) = CA

	ldrb r0,[r2,#1]			// r0 = FA @(fp-17)
	add r0,r0,r5			// FA = FA + SF
	// Salvamos variable local 'FA'
	mov r2,#0xFF
	and r0,r0,r2
	mov r2,r7
	sub r2,#17
	strb r0,[r2]			// @(fp-17) = FA

	add r4,r4,#1			// iter++
	ldr r2,[r7,#8]			// r2 = @tablero
	//ficha_valida_thumb(FA, CA, tablero, &posicion_valida)
	bl ficha_valida_thumb
	b in_thumb
out_thumb:
	//if ((posicion_valida == 1) && (casilla == color))
	cmp r2,#1
	bne ret_0
	cmp r0,r1
	bne ret_0
	mov r0,r4				// return = iter
	b ret_iter
ret_0:
	mov r0,#0				// return = 0
ret_iter:

	//restauramos los valores de la pila
	add sp,sp,#8
	pop {r4-r7}
	pop {r3}
	bx r3

.end
#        END
