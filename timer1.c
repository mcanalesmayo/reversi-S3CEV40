/*
 * timer1.c
 *
 *  Created on: 02/12/2014
 */

#define TIEMPO_PARPADEO 400
#define TIEMPO_TOTAL 5000
/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/
static int num_ms = 0;
static char cancelar = 0;

/*--- declaracion de funciones ---*/
void timer1_ISR(void) __attribute__ ((interrupt ("IRQ")));
void timer1_init(void);
void timer1_stop(void);
extern void ejecutar_parpadeo();
extern void confirmar_mov();
extern void restablecer_mov();

/*--- codigo de las funciones ---*/
void timer1_ISR(void)
{
	if(cancelar == 1){ // se ha pulsado un boton para cancelar
		restablecer_mov();
		timer1_stop();
	}
	else{
		num_ms += 1;
		if(num_ms % TIEMPO_PARPADEO == 0){
			ejecutar_parpadeo();
		}
		if(num_ms == TIEMPO_TOTAL){ // ha llegado al tiempo de expiracion
			confirmar_mov();
			timer1_stop();
		}
	}
	/* borrar bit en I_ISPC para desactivar la solicitud de interrupcion */
	rI_ISPC |= BIT_TIMER1; // BIT_TIMER4 esta definido en 44b.h
}

void cancelar_mov(void){
	cancelar = 1;
}

void timer1_stop(void){
	//Deshabilita interrupciones de timer1
	rINTMSK |= BIT_TIMER1;
	//Para el reloj timer1
	rTCON &= ~(0x100);
}

void timer1_init(void){

	/* Configuraion controlador de interrupciones */
	rINTMSK &= ~(BIT_TIMER1);

	// inicializar variables
	cancelar = 0;
	num_ms = 0;

	/* Establece la rutina de servicio para TIMER1 */
	pISR_TIMER1=(unsigned)timer1_ISR;

	/* Configura el Timer1 */
    rTCFG0 &= ~(0xFF); // valor del preescalado del timer1 = 0 (compartira preescalado con timer0)
    rTCFG1 &= ~(0xF0); // divisor del timer1 = 1/2
    rTCNTB1 = 32000; // valor inicial de cuenta (la cuenta es descendente)
    rTCMPB1 = 0; // valor de comparacion
	rTCON |= 0x200; // activar manual update de los registros TCNTB1 y TCMPB1

    rTCON |= 0x900; // lanza el timer1 con auto reload
	rTCON &= ~(0x200); // Desactiva manual update del timer1
}
