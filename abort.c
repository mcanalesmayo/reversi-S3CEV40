/*
 * abort.c
 *
 *  Created on: 25/11/2014
 */

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/

/*--- declaracion de funciones ---*/
void dabort_ISR(void) __attribute__ ((interrupt ("ABORT")));
void dabort_init(void);

/*--- codigo de las funciones ---*/
void dabort_ISR(void)
{
	D8Led_symbol(14);
	while(1);
}

void dabort_init(void){
	/* Establece la rutina de servicio para DAbort */
	pISR_DABORT=(unsigned)dabort_ISR;
}