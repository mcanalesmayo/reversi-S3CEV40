/*********************************************************************************************
* Fichero:	button.c
* Autor:		
* Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
* Version:	
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "def.h"

/*--- variables globales ---*/

/*--- declaracion de funciones ---*/
void Eint4567_ISR(void) __attribute__ ((interrupt ("IRQ"))); 
void Eint4567_init(void);
extern void push_debug(int val);
extern void timer_init(void);
extern void cancelar_mov(void);

/*--- codigo de funciones ---*/
void Eint4567_init(void){

	/* Configuracion del controlador de interrupciones. Estos registros estan definidos en 44b.h*/
    rI_ISPC    = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
	rEXTINTPND = 0xf;       // Borra EXTINTPND escribiendo 1s en el propio registro    
	rINTMSK    &= ~(BIT_EINT4567); // Enmascara todas las lineas excepto eint4567 y el bit global
		
	/* Establece la rutina de servicio para Eint4567 */
    pISR_EINT4567 = (int)Eint4567_ISR;
    
    /* Configuracion del puerto G */
    rPCONG  = 0xffff;        		// Establece la funcion de los pines (EINT0-7)
	rPUPG   = 0x0;                  // Habilita el "pull up" del puerto	    
	rEXTINT |= 0x22222222;   // Configura las lineas de int. como de flanco de bajada	

    /* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
    rI_ISPC    |= (BIT_EINT4567);
	rEXTINTPND = 0xf;
}

void Eint4567_ISR(void){

	/* Desactivar la interrupcion del pulsador. */
	rINTMSK |= BIT_EINT4567;
    /* Identificar la interrupcion (hay dos pulsadores)*/
    int bot = rEXTINTPND;
    timer_init();
    cancelar_mov();
    //push_debug(5);
    set_pulsador(bot);
    /* Finalizar ISR */
    rEXTINTPND = 0xf;				// borra los bits en EXTINTPND		
    rI_ISPC |= BIT_EINT4567;		// borra el bit pendiente en INTPND
}
