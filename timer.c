/*********************************************************************************************
* Fichero:		timer.c
* Autor:		
* Descrip:		funciones de control del timer0 del s3c44b0x
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- variables globales ---*/
static int num_ms = 0; // milisegundos
static int contador_ficha = 0; //contador de incrementos
static char confirmacion = 0;
static char boton_pulsado = 0; // flag de boton pulsado
typedef enum {IZQUIERDO, DERECHO} boton;
typedef enum {STOP, PULSAR, ESTABLE, SOLTAR} automata_rebotes;
static boton pulsador;
static automata_rebotes estado = STOP;

/*--- declaracion de funciones ---*/
void timer_ISR(void) __attribute__ ((interrupt ("FIQ")));
void timer_init(void);
void timer_stop(void);
int get_contador_ficha(void);
char get_confirmacion(void);
void decrementar_contador_ficha(void);
void set_confirmacion(char val);
extern int get_simbolo(void);

/*--- codigo de las funciones ---*/
void set_pulsador(char v){
	if (v == 4){
		pulsador = IZQUIERDO;
	}
	else if (v == 8){
		pulsador = DERECHO;
	}
}

char get_confirmacion(void){
	return confirmacion;
}

void set_confirmacion(char val){
	confirmacion = val;
}

int get_contador_ficha(void){
	return contador_ficha;
}

void decrementar_contador_ficha(void){
	contador_ficha -= 1;
}

void boton_esperar_pulsacion(void)
{
	boton_pulsado = 0;
	while(boton_pulsado==0);
	contador_ficha = 0;
}

void timer_ISR(void)
{
	switch (estado){
	case STOP:
		if(pulsador == DERECHO && get_simbolo() != 15 && get_simbolo() != 12){
			confirmacion = 1;
		}
		boton_pulsado = 1;
	    rEXTINTPND = 0xf; // borra los bits en EXTINTPND
		rI_ISPC |= BIT_EINT4567; // Se borra el bit pendiente de la interrupcion del boton.
		rINTMSK &= ~(BIT_EINT4567); // Activa la interrupcion del boton ya que se ha soltado.
		timer_stop();
		break;
	case PULSAR:
		num_ms += 1;
		if (num_ms == 100){
			switch (pulsador){
			case IZQUIERDO:
				contador_ficha+=1;
				break;
			case DERECHO:
				num_ms = 0;
				break;
			}
			estado = ESTABLE;
		}
		break;
	case ESTABLE:
		switch (pulsador){
		case IZQUIERDO: //boton izq (0100)
			num_ms += 1;
			if(num_ms % 500 == 0){
				contador_ficha+=1;
			}
			if (rPDATG & (1 << 6)){ //si se ha soltado
				estado = SOLTAR;
				num_ms = 0;
			}
			break;
		case DERECHO: //boton der (1000)
			if (rPDATG & (1 << 7)){ //si se ha soltado
				estado = SOLTAR;
			}
			break;
		}
		break;
	case SOLTAR:
		num_ms += 1;
		if (num_ms == 100){
			estado = STOP;
			num_ms = 0;
		}
		break;
	}

	/* borrar bit en I_ISPC para desactivar la solicitud de interrupcion */
    rF_ISPC |= BIT_TIMER0; // BIT_TIMER0 esta definido en 44b.h y pone un uno en el bit 13 que correponde al Timer0
}

void timer_stop(void){
	//Deshabilita interrupciones de timer0
	rINTMSK |= BIT_TIMER0;
	//Para el reloj timer0
	rTCON &= ~(0x1);
}

void timer_init(void){

	estado = PULSAR;

	/* Configuraion controlador de interrupciones */
	rINTMOD |= BIT_TIMER0; // Habilita FIQ para el timer0.
	rINTMSK &= ~(BIT_TIMER0);

	/* Establece la rutina de servicio para TIMER0 */
	pISR_FIQ=(unsigned)timer_ISR;

	/* Configura el Timer0 */
    rTCFG0 &= ~(0xFF); // preescalado del timer0 = 0 (compartira preescalado con timer1) 
    rTCFG1 &= ~(0xF); // divisor del timer0 = 1/2
    rTCNTB0 = 32000; // valor inicial de cuenta (la cuenta es descendente)
    rTCMPB0 = 0; // valor de comparacion
	rTCON |= 0x2; // activar manual update de los registros TCNTB0 y TCMPB0

    rTCON |= 0x9; // lanza el timer0 con auto reload
    rTCON &= ~(0x2); // Desactiva manual update del timer4
}
