#define VOLTEA 1
#define NO_VOLTEA 0
#define TAM_TABLERO 8

// usamos tabla_valor como variable global para que sea mas facil encontrarlas en el simulador
// almacenamos el tablero en un 8x8.
char tabla_valor[TAM_TABLERO][TAM_TABLERO] = {
        8,2,7,3,3,7,2,8,
        2,1,4,4,4,4,1,2,
        7,4,6,5,5,6,4,7,
        3,4,5,0,0,5,4,3,
        3,4,5,0,0,5,4,3,
        7,4,6,5,5,6,4,7,
        2,1,4,4,4,4,1,2,
        8,2,7,3,3,7,2,8};
// candidatos indica las posiciones a explorar, se usa para no explorar todo el tablero innecesariamente
// 0 casilla no alcanzable, 1 casilla a explorar, 2 casilla ya evaluada.
char candidatos[TAM_TABLERO][TAM_TABLERO] = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,0,1,0,0,1,0,0,
        0,0,1,0,0,1,0,0,
        0,0,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0};

/* Variables globales */
typedef enum {FILA, COLUMNA, CONFIRMAR} ficha;
static ficha actual=FILA;
static int simbolo;
static char fila = 0;
static char columna = 0;
static char mov_done = 0; // flag de movimiento realizado
static char reset_mov = 0; // flag de resetear el movimiento
static char parpadear = 0; // flag de parpadear
static char parpadeo = 0; // 0 = texto esta oculto; 1 = texto esta mostrado
static int tiempo; // tiempo transcurrido durante la partida (en decimas)
static char actualizar_tiempo = 0; // flag de actualizar tiempo

/* Funciones globales */
extern int patron_volteo_arm(char f, char c, char SF, char SC, char color, char tablero[][TAM_TABLERO]);
extern int patron_volteo_thumb(char f, char c, char SF, char SC, char color, char tablero[][TAM_TABLERO]);
extern int timer2_leer(void);
extern void timer1_init(void);
extern int get_contador_ficha(void);
extern void decrementar_contador_ficha(void);
extern char get_confirmacion(void);
extern void set_confirmacion(char val);
void incrementar0_8(void);
extern void D8Led_symbol(int val);
int get_simbolo(void);
void set_simbolo(int val);
extern void display_ficha_jugador(int fila, int columna);
extern void display_ficha_cpu(int fila, int columna);
extern void undisplay_ficha(int fila, int columna);
void actualizar_lcd(char tablero[TAM_TABLERO][TAM_TABLERO], int blancas, int negras);
extern void display_resultado_actual(int blancas, int negras);
extern void undisplay_mensaje_ayuda();
extern void display_mensaje_ayuda();
extern void display_tiempo();
extern void display_actualizar_tiempo(int tiempo);
extern void display_mensaje_ayuda();
extern void display_final(int blancas, int negras);
extern void timer4_init();
extern void timer4_stop();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 indica vacio, 1 indica blanco y 2 indica negro
// pone el tablero a cero y luego coloca las fichas centrales.
void init_table(char tablero[][TAM_TABLERO])
{
    int i,j;
    for (i=0; i<TAM_TABLERO; i++) {
        for (j=0; j<TAM_TABLERO; j++) {
            tablero[i][j] = 0;
           }
    }
    tablero[3][3] = 1;
    tablero[4][4] = 1;
    tablero[3][4] = 2;
    tablero[4][3] = 2;
    candidatos[3][3] = 2;
    candidatos[4][4] = 2;
    candidatos[3][4] = 2;
    candidatos[4][3] = 2;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// espera a que ready valga 1. CUIDADO: si el compilador lo coloca en un registro no funcionara. Si pasa eso habra que definirla como volatile para que no lo meta en un registro sino que lo busque en memoria cada vez
void set_simbolo(int val){
	simbolo = val;
}

int get_simbolo(){
	return simbolo;
}

void incrementar0_8(void){
	if(simbolo==12 || simbolo==15){
		simbolo = 0;
	}
	else{
		simbolo += 1;
		if(simbolo==9){
			simbolo = 0;
		}
	}

	D8Led_symbol(simbolo);
}

void esperar_mov()
{
	while(mov_done == 0){
		undisplay_ficha(8,8);
		display_mensaje_ayuda();
		if (reset_mov == 1){ // si viene de reset mov -> borrar ficha del lcd
			simbolo = 15;
			D8Led_symbol(15);
			undisplay_ficha(fila,columna);
			reset_mov = 0;
		}
		while (actual == FILA) { // bucle de espera de respuestas hasta que el se modifique el valor de ready (hay que hacerlo manualmente)
			if(actualizar_tiempo == 1){ // actualiza cada decima de segundo
				tiempo += 1;
				display_actualizar_tiempo(tiempo);
				actualizar_tiempo = 0;
			}
			while (get_contador_ficha() > 0){
				incrementar0_8();
				decrementar_contador_ficha();
			}
			if(get_confirmacion()==1){ // si se pulsa el boton derecho.
				fila = (char) simbolo;
				simbolo = 12;
				D8Led_symbol(12);
				set_confirmacion(0);
				actual = COLUMNA;
			}
		}

		while (actual == COLUMNA) { // bucle de espera de respuestas hasta que el se modifique el valor de ready (hay que hacerlo manualmente)
			if(actualizar_tiempo == 1){ // actualiza cada decima de segundo
				tiempo += 1;
				display_actualizar_tiempo(tiempo);
				actualizar_tiempo = 0;
			}
			while (get_contador_ficha() > 0){
				incrementar0_8();
				decrementar_contador_ficha();
			}
			if(get_confirmacion()==1){ // si se pulsa el boton derecho.
				columna = (char) simbolo;
				simbolo = 15;
				D8Led_symbol(15);
				set_confirmacion(0);
				actual = CONFIRMAR;
				timer1_init();
			}
		}
		
		if (fila == 8 || columna == 8){ // si pasar movimiento -> ocultar mensaje de ayuda para mostrar mensaje de pasando
			undisplay_mensaje_ayuda();
		}
		while (actual == CONFIRMAR){
			if(parpadear == 1){ // ejecutar parpadeo
				if(parpadeo == 0){ // estaba en blanco -> mostrar ficha
					display_ficha_jugador(fila, columna);
					parpadeo = 1;
				}
				else{ // estaba mostrada -> esconder ficha
					undisplay_ficha(fila,columna);
					parpadeo = 0;
				}
				parpadear = 0;
			}
		}
	}
	mov_done = 0;
}

void actualizar_tiempo_transcurrido(){
	actualizar_tiempo = 1;
}

void ejecutar_parpadeo(){
	parpadear = 1;
}

void confirmar_mov(){ // confirmar el movimiento
	mov_done = 1;
	actual = FILA;
}

void restablecer_mov(){ // modificar el movimiento
	reset_mov = 1;
	actual = FILA;
}

void actualizar_lcd(char tablero[TAM_TABLERO][TAM_TABLERO], int blancas, int negras){
	int i;
	int j;
	for (i=0;i<TAM_TABLERO;i++){
		for (j=0;j<TAM_TABLERO;j++){
			if(tablero[i][j] == 1){ // ficha cpu
				undisplay_ficha(i,j);
				display_ficha_cpu(i,j);
			}
			else if (tablero[i][j] == 2){ // ficha jugador
				display_ficha_jugador(i,j);
			}
		}
	}

	display_resultado_actual(blancas, negras);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IMPORTANTE: AL SUSTITUIR FICHA_VALIDA Y patron_volteo POR RUTINAS EN ENSAMBLADOR HAY QUE RESPETAR LA MODULARIDAD.
//  DEBEN SEGUIR SIENDO LLAMADAS A FUNCIONES Y DEBEN CUMPLIR CON EL ATPCS (VER TRANSPARENCIAS Y MATERIAL DE PRACTICAS):
//  - DEBEN PASAR LOS PARAMETROS POR LOS REGISTROS CORRESPONDIENTES
//  - GUARDAR EN PILA SOLO LOS REGISTROS QUE TOCAN
//  - CREAR UN MARCO DE PILA TAL Y COMO MUESTRAN LAS TRANSPARENCIAS DE LA ASIGNATURA (CON EL PC, FP, LR,....)
//  - EN EL CASO DE LAS VARIABLES LOCALES, SOLO HAY QUE APILARLAS SI NO SE PUEDEN COLOCAR EN UN REGISTRO.
//    SI SE COLOCAN EN UN REGISTRO NO HACE FALTA NI GUARDARLAS EN PILA NI RESERVAR UN ESPACIO EN LA PILA PARA ELLAS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// devuelve el contenido de la posicion indicadas por la fila y columna actual.
// Ademas informa si la posicion es valida y contiene alguna ficha. Esto lo hace por referencia (en *posicion_valida).
// Si devuelve un 0 no es valida o esta vacia.
char ficha_valida_c(char fila_actual, char columna_actual, char tablero[][TAM_TABLERO], int *posicion_valida)
{
    char ficha;
    ficha = tablero[fila_actual][columna_actual];
    if ((fila_actual > 7) ||
    		(fila_actual < 0) ||
    		(columna_actual > 7) ||
    		(columna_actual < 0) ||
    		(ficha == 0)) {
        *posicion_valida = 0;
    }
    else {
        *posicion_valida = 1;
    }
    return ficha;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Comprueba si hay que actualizar una determinada direccion, para ello busca el patron de volteo:
// n fichas del rival seguidas de una ficha del jugador actual. SF y SC son las cantidades a sumar para movernos en la direccion
// que toque. Color indica el color de la pieza que se acaba de colocar. La funcion devuelve el numero de fichas que habra que voltear.
// f y c son la fila y columna a analizar.
int patron_volteo_c(char f, char c, char SF, char SC, char color, char tablero[][TAM_TABLERO]){
    char FA, CA, casilla; //FA y CA indican la posicion actual. casilla es la casilla que se lee del tablero
    int iter; // indica el numero de iteraciones. Se usa para saber cuantas fichas habra que voltear
    int posicion_valida; // indica si la posicion es valida y contiene una ficha de algun jugador
    iter = 0;
    FA = f + SF;
    CA = c + SC;
    casilla = ficha_valida_c(FA, CA, tablero, &posicion_valida);
    while ((posicion_valida == 1) && (casilla != color)) {
    	// mientras la casilla esta en el tablero, no esta vacia, y es del color rival seguimos buscando el patron de volteo.
        FA = FA + SF;
        CA = CA + SC;
        iter++;
        casilla = ficha_valida_c(FA, CA, tablero, &posicion_valida);
    }
    if ((posicion_valida == 1) && (casilla == color)) {
    	//si la ultima posicion era valida y la ficha es del jugador actual hemos encontrado el patron.
        return iter; // devolvemos el numero de iteraciones
    }
    else{
        return 0; // devolvemos 0 que quiere decir patron no encontrado.
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Voltea n fichas en la direccion que toque
// SF y SC son las cantidades a sumar para movernos en la direcci?n que toque
// color indica el color de la pieza que se acaba de colocar
// f y c son la fila y columna a analizar
void voltear(char f, char c, char SF, char SC, int n, char color, char tablero[][TAM_TABLERO]){
    int i;
    char FA, CA; //FA y CA indican la posici?n actual
    FA=f;
    CA=c;
    for (i=0; i<n; i++){
        FA = FA + SF;
        CA = CA + SC;
        tablero[FA][CA] = color;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// comprueba si hay que actualizar alguna ficha
// no comprueba que el movimiento realizado sea v?lido
// f y c son la fila y columna a analizar
int actualizar_tablero(char f, char c, char color, char tablero[][TAM_TABLERO]){
    char SF, SC; //SF y SC son las cantidades a sumar para movernos en la direcci?n que toque
    int i, flip;
    for (i=0;i<TAM_TABLERO;i++){ // 0 es norte, 1 NE, 2 E...
        switch (i) {
        case 0: {SF = -1;   SC = 0;     break;}
        case 1: {SF = -1;   SC = 1;     break;}
        case 2: {SF = 0;    SC = 1;     break;}
        case 3: {SF = 1;    SC = 1;     break;}
        case 4: {SF = 1;    SC = 0;     break;}
        case 5: {SF = 1;    SC = -1;    break;}
        case 6: {SF = 0;    SC = -1;    break;}
        case 7: {SF = -1;   SC = -1;    break;}
        default: {
        //  printf("error en actualizar_tablero \n");
            return 0;
            }
        }
        //int antes = timer2_leer();
        flip = patron_volteo_c(f, c, SF, SC,color, tablero); // flip indica las fichas a voltear
        //int despues = timer2_leer();
        //despues = despues - antes;
    //  printf("%d \n", flip);
        voltear(f, c, SF, SC, flip, color, tablero);
    };
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// recorre todo el tablero comprobando en cada posici?n si se pued emover. En tal caso mira la puntuaci?n de la posici?n y si es la mejor que se ha encontrado la guarda
// al acabar escribe el movimiento seleccionado en f y c
int elegir_mov(char* f, char* c, char tabla_valor[][TAM_TABLERO], char tablero[][TAM_TABLERO]){
    int i, j, k, found;
    int mf=-1; //almacena la fila del mejor movimiento encontrado
    int mc; //almacena la columna del mejor movimiento encontrado
    char mejor=0; // almacena el mejor valor encontrado
    int patron;
    char SF, SC; //SF y SC son las cantidades a sumar para movernos en la direcci?n que toque
    for (i=0;i<TAM_TABLERO;i++){ //recorremos todo el tablero comprobando d?nde podemos mover. Miramos la puntuaci?n de los movimientos encontrados y nos quedamos con el mejor
        for (j=0;j<TAM_TABLERO;j++){
            if (candidatos[i][j]==1){ // candidatos es una variable global que ?ndica en qu? casillas quiz? se pueda mover.
                if (tablero[i][j]==0){
                found =0;
                k=0;
                while ((found==0)&&(k<TAM_TABLERO)){  // en este bucle comprobamos si es un movimiento v?lido (es decir si iomplica voltear en alguna direcci?n
                        switch (k) {                //k representa la direcci?n que miramos 1 es norte, 2 NE, 3 E...
                            case 0: {SF=-1; SC=0; break;}
                            case 1: {SF=-1; SC=1; break;}
                            case 2: {SF=0; SC=1; break;}
                            case 3: {SF=1; SC=1; break;}
                            case 4: {SF=1; SC=0; break;}
                            case 5: {SF=1; SC=-1; break;}
                            case 6: {SF=0; SC=-1; break;}
                            case 7: {SF=-1; SC=-1; break;}
                            default: {
                            //  printf("error en elegir_mov /n");
                                return 0;
                            }
                        }
                        patron = patron_volteo_c(i, j, SF, SC,1, tablero); // comprobar nos dice qu? hay que voltear en cada direcci?n
                    //  printf("%d ", flip);
                        if (patron>0){
                            found =1;
                            if (tabla_valor[i][j]> mejor){
                                mf=i;
                                mc=j;
                                mejor=tabla_valor[i][j];}
                        }
                        k++; // si no hemos encontrado nada probamos con la siguiente direcci?n
                        }
                    }
                }
            }
    }
    *f= (char)mf;
    *c= (char)mc;
    return mf;// si no se ha encontrado una posici?n v?lida devuelve -1
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// cuenta el n?mero de fichas de cada color y lo guarda en la direcci?n b (blancas) y n (negras)
void contar(int* b, int* n, char tablero[][TAM_TABLERO])
{
    int i,j;

    *b = 0;
    *n = 0;
    for (i=0; i<TAM_TABLERO; i++) { //recorremos todo el tablero contando las fichas de cada color
        for (j=0; j<TAM_TABLERO; j++) {
            if (tablero[i][j] == 1) { (*b)++; }
            else if (tablero[i][j] == 2) { (*n)++; }
        }
    }
}

void actualizar_candidatos(char f, char c)
{
    candidatos[f][c] = 2; // donde ya se ha colocado no se puede volver a colocar. En las posiciones alrededor si
    if (f > 0) {
        if (candidatos[f-1][c] != 2){ candidatos[f-1][c] = 1;}
        if ((c > 0)&&(candidatos[f-1][c-1] != 2)) { candidatos[f-1][c-1] = 1; }
        if ((c < 7)&&(candidatos[f-1][c+1] != 2)) { candidatos[f-1][c+1] = 1; }
    }
    if (f < 7) {
        if (candidatos[f+1][c] != 2){candidatos[f+1][c] = 1;}
        if ((c > 0)&&(candidatos[f+1][c-1] != 2)) { candidatos[f+1][c-1] = 1; }
        if ((c < 7)&&(candidatos[f+1][c+1] != 2)) { candidatos[f+1][c+1] = 1; }
    }
    if ((c > 0)&&(candidatos[f][c-1] != 2)) { candidatos[f][c-1] = 1; }
    if ((c < 7)&&(candidatos[f][c+1] != 2)) { candidatos[f][c+1] = 1; }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// proceso principal del juego que recibe el tablero, y las direcciones en las que indica el jugador la fila y la columna y la se?al de ready que indica que se han actualizado fila y columna
void reversi6(char tablero[][TAM_TABLERO])  // en esta versi?n el usuario lleva negras y el ordenador blancas
{   // no se comprueba que el jugador mueva bien. S?lo que el ordenador realice un movimiento correcto.
  int done; // nos indica si se ha conseguido mover
  int move = 0; //indica que el jugador ha movido;
  tiempo = 0; // inicializa el valor del tiempo transcurrido a 0
  display_tiempo(); // mostrar mensaje de tiempo
  display_actualizar_tiempo(tiempo); // mostrar el valor del tiempo
  display_mensaje_ayuda(); // mostrar mensaje de ayuda
  int fin = 0;// fin se pone a 1 si el jugador no ha podido mover (ha introducido un valor de movimiento con alg?n 6) y luego el ordenador tampoco puede.
  int blancas = 2, negras = 2; // se usan para contar el n?mero de fichas de cada color.
  char f,c; // fila y columna elegidas por el programa para su movimiento

  init_table(tablero);
  actualizar_lcd(tablero, blancas, negras);
  timer4_init(); // comienza el contador de tiempo transcurrido
  while (fin == 0) {
    move = 0;
    esperar_mov();
    if (((fila) != TAM_TABLERO) && ((columna) != TAM_TABLERO)) { // si la fila o columna son 8 asumimos que el jugador no puede mover
        tablero[fila][columna] = 2;
        actualizar_tablero(fila, columna, 2, tablero);
        contar(&blancas, &negras, tablero);
        actualizar_lcd(tablero, blancas, negras);
        actualizar_candidatos(fila, columna);
        move = 1;
    }
    done = elegir_mov(&f, &c, tabla_valor, tablero); //escribe el movimiento en las variables globales fila columna
    if (done == -1 && move == 0){
       fin = 1;
    }
    else {
        tablero[f][c] = 1;
        actualizar_tablero(f, c, 1, tablero);
        contar(&blancas, &negras, tablero);
        actualizar_lcd(tablero, blancas, negras);
        actualizar_candidatos(f, c);
    }
  }
  timer4_stop(); // parar reloj del tiempo transcurrido
  display_final(blancas,negras); // mostrar resultado final
}
