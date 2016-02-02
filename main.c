/*********************************************************************************************
* Fichero:	main.c
* Autor:	
* Descrip:	punto de entrada de C
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "stdio.h"

/*--- variables globales ---*/

/*--- funciones externas ---*/
extern void dabort_init();
extern void timer2_init();
extern void Eint4567_init();
extern void reversi6(char tablero[][8]);
extern void D8Led_init(void);
extern void D8Led_symbol(int value);
extern void boton_esperar_pulsacion(void);
extern void set_simbolo(int val);
extern void push_debug_abort();
extern void Lcd_Start();

/*--- declaracion de funciones ---*/
void Main(void);

/*--- codigo de funciones ---*/
void Main(void)
{
   /* Inicializa controladores */
    sys_init();        // Inicializacion de la placa, interrupciones y puertos
    dabort_init();		// Inicializacion de la excepcion Data Abort
	timer2_init();		// Inicializacion del timer2
	Eint4567_init();	// inicializamos los pulsadores. Cada vez que se pulse se vera reflejado en el 8led
	D8Led_init(); 		// inicializamos el 8led

	double foo = 123; //Para visualizar correctamente el tablero en memoria.
	char tablero[8][8];

	D8Led_symbol(16);
	Lcd_Start();
	boton_esperar_pulsacion();
	while (1){
		//estado inicial
		set_simbolo(15);
		D8Led_symbol(15);

		reversi6 (tablero);

		D8Led_symbol(16);
		boton_esperar_pulsacion();
		Lcd_Start();
	}

	/* Calibracion del reloj */
	/*int antes = timer2_leer();
	Delay(50000);
	int despues = timer2_leer();
	int resta = despues - antes;

	int x =1;*/

	/*
	while(1){
		Delay(50000);
	}
	*/

}
