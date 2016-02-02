/*
 * pila.c
 *
 *  Created on: 28/10/2014
 */

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "stdio.h"

/*--- variables globales ---*/

/*--- funciones externas ---*/
extern int timer2_leer();

/*--- declaracion de funciones ---*/

/*--- codigo de funciones ---*/
void push_debug(int data){
	static int *dirInit = (int *) 0x0C7FEF00;
	static int *dirTope = (int *) 0x0C7FDF00;
	static int *dirActual = (int *) 0x0C7FEF00;
	//Se pueden insertar hasta 2^9 datos.
	if (dirActual == dirTope){
		dirActual = dirInit;
	}
	//Primera posicion para el dato
	dirActual -= 1;
	*dirActual = data;
	//Segunda posicion para el valor del reloj
	dirActual -= 1;
	*dirActual = timer2_leer();
}

void push_debug_abort(){
	__asm__("ldr r0, =0x0C7FEF01");
	__asm__("mov r1, #5");
	__asm__("str r1,[r0]");
}