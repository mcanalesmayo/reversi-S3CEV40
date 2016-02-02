/*********************************************************************************************
* Fichero:		timer2.c
* Autor:		
* Descrip:		funciones de control del timer2 del s3c44b0x
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/
static int num_interrupts = 0;

/*--- declaracion de funciones ---*/
void timer2_ISR(void) __attribute__ ((interrupt ("IRQ")));
void timer2_init(void);
void timer2_start(void);
int timer2_leer(void);

/*--- codigo de las funciones ---*/
void timer2_ISR(void)
{
	num_interrupts += 1;
	/* borrar bit en I_ISPC para desactivar la solicitud de interrupcion */
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 esta definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2.
}

void timer2_start(void)
{
	num_interrupts = 0;
}

int timer2_leer(void)
{
	/*
	 * Deshabilita la interrupcion del timer2 para mantener la consistencia en
	 * los valores que se van a leer de los registros del reloj.
	 */
	rINTMSK |= BIT_TIMER2;
	int val_actual = rTCNTO2;
	int n_inter = num_interrupts;
	//Habilita la interrupcion del timer2.
	rINTMSK &= ~(BIT_TIMER2);

	return (rTCNTB2-val_actual)/(MCLK/2000000) + n_inter*(rTCNTB2/(MCLK/2000000)); //microsegundos
}

void timer2_init(void){

	/* Configuraion controlador de interrupciones */
	rINTMSK &= ~(BIT_TIMER2);

	/* Establece la rutina de servicio para TIMER2 */
	pISR_TIMER2=(unsigned)timer2_ISR;

	/* Configura el Timer2 */
    rTCFG0 &= ~(0xFF00); // valor del preescalado del timer2 = 0 (compartira preescalado con timer3)
    rTCFG1 &= ~(0xF00); // divisor del timer2 = 1/2
    rTCNTB2 = 65535; // valor inicial de cuenta (la cuenta es descendente)
    rTCMPB2 = 0; // valor de comparacion
	rTCON |= 0x2000;  // activar manual update de los registros TCNTB2 y TCMPB2

    rTCON |= 0x9000; // lanza el timer2 con auto reload
	rTCON &= ~(0x2000); // Desactiva manual update del timer2
}
