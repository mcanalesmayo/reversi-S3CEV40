/*
 * timer4.c
 */

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/

/*--- declaracion de funciones ---*/
void timer4_ISR(void) __attribute__ ((interrupt ("IRQ")));
void timer4_init(void);
void timer4_stop(void);
extern void actualizar_tiempo_transcurrido();

/*--- codigo de las funciones ---*/
void timer4_ISR(void)
{
	actualizar_tiempo_transcurrido();
	rI_ISPC |= BIT_TIMER4; // limpiar bit de pendiente del timer4
}

void timer4_stop(void){
	//Deshabilita interrupciones de timer4
	rINTMSK |= BIT_TIMER4;
	//Para el reloj timer4
	rTCON &= ~(0x100000);
}

void timer4_init(void){ // iniciar timer4 para interrumpir cada décima de segundo

	/* Configuraion controlador de interrupciones */
	rINTMSK &= ~(BIT_TIMER4);

	/* Establece la rutina de servicio para TIMER4 */
	pISR_TIMER4=(unsigned)timer4_ISR;

	/* Configura el Timer4 */
    rTCFG0 |= 0xFF0000; // preescalado del timer4 = 255 (compartirá preescalado con timer5)
    rTCFG1 &= ~(0xF0000); // divisor del timer4 = 1/2
    rTCNTB4 = 12500; // valor inicial de cuenta (la cuenta es descendente)
    rTCMPB4 = 0;// valor de comparacion
	rTCON |= 0x200000; // activar manual update de los registros TCNTB4 y TCMPB4

    rTCON |= 0x900000; // lanza el timer4 con auto reload
	rTCON &= ~(0x200000); // Desactiva manual update del timer4
}
