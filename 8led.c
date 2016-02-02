/*********************************************************************************************
* Fichero:	8led.c
* Autor:		
* Descrip:	Funciones de control del display 8-segmentos
* Version:	
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44b.h"
#include "44blib.h"

/*--- definicion de macros ---*/
/* Mapa de bits de cada segmento 
  (valor que se debe escribir en el display para que se ilumine el segmento) */
#define cero			~0xED		
#define uno				~0x60		
#define dos				~0xCE
#define tres			~0xEA
#define cuatro			~0x63
#define cinco			~0xAB
#define seis			~0x2F
#define siete			~0xE0
#define ocho			~0xEF
#define nueve			~0xE3
#define A				~0xE7
#define B				~0x2F
#define C				~0x8D
#define D				~0x6E
#define E				~0x8F
#define F				~0x87
#define blank			~0x00		

/*--- variables globales ---*/
/* tabla de segmentos */
static int Symbol[] = { cero, uno, dos, tres, cuatro, cinco, seis, siete, ocho, nueve, A, B, C, D, E, F, blank};

			  					  
/*--- declaracion de funciones ---*/
void D8Led_init(void);
void D8Led_symbol(int value); 

/*--- codigo de las funciones ---*/
void D8Led_init(void)
{
    /* Estado inicial del display con todos los segmentos iluminados 
       (buscar en los ficheros de cabera la direccion implicada) */
    LED8ADDR = 0;
}

void D8Led_symbol(int value)
{
	/* muestra el Symbol[value] en el display (analogo al caso anterior) */
	if((value >= 0) && (value < 17)){
		LED8ADDR = Symbol[value];
	}
}
