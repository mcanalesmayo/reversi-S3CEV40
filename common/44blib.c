#include "44b.h"
#include "44blib.h"
#include "def.h"
#include "option.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define STACKSIZE    0xa00 //SVC satck size(do not use user stack)//
#define HEAPEND     (_ISR_STARTADDRESS-STACKSIZE-0x500) // = 0xc7ff000//
//SVC Stack Area:0xc(e)7ff000-0xc(e)7ffaff//

extern char Image_RW_Limit[];
volatile unsigned char *downPt;
unsigned int fileSize;
void *mallocPt=Image_RW_Limit;

void (*restart)(void)=(void (*)(void))0x0;
void (*run)(void)=(void (*)(void))DOWNLOAD_ADDRESS;

//--------------------------------SYSTEM---------------------------------//
static int delayLoopCount=400;

void Delay(int time)
// time=0: adjust the Delay function by WatchDog timer.//
// time>0: the number of loop time//
// 100us resolution.//
{
	int i,adjust=0;
	if(time==0)
	{
		time=200;
		adjust=1;
		delayLoopCount=400;
		rWTCON=((MCLK/1000000-1)<<8)|(2<<3);	// 1M/64,Watch-dog,nRESET,interrupt disable//
		rWTDAT=0xffff;
		rWTCNT=0xffff;	 
		rWTCON=((MCLK/1000000-1)<<8)|(2<<3)|(1<<5); // 1M/64,Watch-dog enable,nRESET,interrupt disable //
	}
	for(;time>0;time--)
		for(i=0;i<delayLoopCount;i++);
	if(adjust==1)
	{
		rWTCON=((MCLK/1000000-1)<<8)|(2<<3);
		i=0xffff-rWTCNT;   //  1count/16us?????????//
		delayLoopCount=8000000/(i*64);	//400*100/(i*64/200)   //
	}
}

void DelayMs(int ms_time)
{
	int i;
	
	for( i = 0 ; i < 1000*ms_time ; i++ )
		;
}

void DelayTime(int num)
{
	int i;
	
	for( i = 0 ; i < num ; i++ )
		;
}

//------------------------PORTS------------------------------//
void Port_Init(void)
{
	//CAUTION:Follow the configuration order for setting the ports. 
	// 1) setting value 
	// 2) setting control register 
	// 3) configure pull-up resistor.  

	//16bit data bus configuration  

	// PORT A GROUP
	// BIT 	9	8	7	6	5	4	3	2	1	0
	// 		A24	A23	A22	A21	A20	A19	A18	A17	A16	A0
	//		0	1	1	1	1	1	1	1	1	1
	rPCONA = 0x1ff;	

	// PORT B GROUP
	// BIT 	10		9		8		7		6		5		4		3		2		1		0
	//		/CS5	/CS4	/CS3	/CS2	/CS1	GPB5	GPB4	/SRAS	/SCAS	SCLK	SCKE
	//		EXT		NIC		USB		IDE		SMC		NC		NC		Sdram	Sdram	Sdram	Sdram
	//      1, 		1,   	1,   	1,    	1,    	0,       0,     1,    	1,    	1,   	1	
	rPDATB = 0x7ff;				// P9-LED1 P10-LED2
	rPCONB = 0x1cf;
    
	// PORT C GROUP
	// BUSWIDTH=16
	//  PC15	14		13		12		11		10		9		8
	//	I		I		RXD1	TXD1	I		I		I		I
	//	NC		NC		Uart1	Uart1	NC		NC		NC		NC
	//	00		00		11		11		00		00		00		00

	//  PC7		6		5		4		3		2		1		0
	//   I		I		I		I		I		I		I		I
	//   NC		NC		NC		NC		IISCLK	IISDI	IISDO	IISLRCK
	//   00		00		00		00		11		11		11		11
	rPDATC = 0xff00;
	rPCONC = 0x0ff0ffff;	
	rPUPC  = 0x30ff;	//PULL UP RESISTOR should be enabled to I/O

	// PORT D GROUP
	// PORT D GROUP(I/O OR LCD)
	// BIT7		6		5		4		3		2		1		0
	//	VF		VM		VLINE	VCLK	VD3		VD2		VD1		VD0
	//	00		00		00		00		00		00		00		00
	rPDATD= 0xff;
	rPCOND= 0xaaaa;	
	rPUPD = 0x0;
	// These pins must be set only after CPU's internal LCD controller is enable
	
	// PORT E GROUP 
	// Bit	8		7		6		5		4		3		2		1		0
	//  	CODECLK	LED4	LED5	LED6	LED7	BEEP	RXD0	TXD0	LcdDisp
	//  	10		01		01		01		01		01		10		10		01
	rPDATE	= 0x1ff;
	rPCONE	= 0x25529;	
	rPUPE	= 0x6;
	
	// PORT F GROUP
	// Bit8		7		6		5		 4		3		2		1		0
	// IISCLK	IISDI	IISDO	IISLRCK	Input	Input	Input	IICSDA	IICSCL
	// 100		100		100		100		00		00		00		10		10
	rPDATF = 0x0;
	rPCONF = 0x252a;
	rPUPF  = 0x0;

	// PORT G GROUP
	// BIT7		6		5		4		3		2		1		0
	// 	INT7	INT6	INT5	INT4	INT3	INT2	INT1	INT0
	//	S3		S4		S5		S6		NIC		EXT		IDE		USB
	//	11      11      11      11      11      11      11      11
	rPDATG = 0xff;
	rPCONG = 0xffff;
	rPUPG  = 0x0;		//should be enabled  
	rSPUCR = 0x7;  		//D15-D0 pull-up disable

	/* Non Cache area */
	rNCACHBE0=((Non_Cache_End>>12)<<16)|(Non_Cache_Start>>12); 
	/* Low level default */
	rEXTINT=0x0;
}

//------------------------UART------------------------------//
void Uart_Init(int mclk,int baud)
{
    int i;
    if(mclk==0)
		mclk=MCLK;

    rUFCON0=0x0;     //FIFO disable
    rUFCON1=0x0;
    rUMCON0=0x0;
    rUMCON1=0x0;
//UART0
    rULCON0=0x3;     //Normal,No parity,1 stop,8 bit
    rUCON0=0x245;    //rx=edge,tx=level,disable timeout int.,enable rx error int.,normal,interrupt or polling
    rUBRDIV0=( (int)(mclk/16./baud + 0.5) -1 );
//UART1
    rULCON1=0x3;
    rUCON1=0x245;
    rUBRDIV1=( (int)(mclk/16./baud + 0.5) -1 );

    for(i=0;i<100;i++);
}

//------------------------ Timer ------------------------------//
void Timer_Start(int divider)  //0:16us,1:32us 2:64us 3:128us
{
    rWTCON=((MCLK/1000000-1)<<8)|(divider<<3);
    rWTDAT=0xffff;
    rWTCNT=0xffff;   

    // 1/16/(65+1),nRESET & interrupt  disable
    rWTCON=((MCLK/1000000-1)<<8)|(divider<<3)|(1<<5);	
}


int Timer_Stop(void)
{
    rWTCON=((MCLK/1000000-1)<<8);
    return (0xffff-rWTCNT);
}

void Beep(int BeepStatus)
{
	if (BeepStatus==0)
		rPDATE=rPDATE|0x8;
	else
		rPDATE=rPDATE&0x1f7;

}

//------------------------ PLL ------------------------------//
void ChangePllValue(int mdiv,int pdiv,int sdiv)
{
    rPLLCON=(mdiv<<12)|(pdiv<<4)|sdiv;
}

//------------------------ General Library ------------------------------//
void * malloc(unsigned nbyte) 
/*Very simple; Use malloc() & free() like Stack*/
//void *mallocPt=Image$$RW$$Limit;
{
    void *returnPt=mallocPt;

    mallocPt= (int *)mallocPt+nbyte/4+((nbyte%4)>0); //to align 4byte

    if( (int)mallocPt > HEAPEND )
    {
		mallocPt=returnPt;
		return NULL;
    }
    return returnPt;
}

void free(void *pt)
{
    mallocPt=pt;
}

void Cache_Flush(void)
{
    int i,saveSyscfg;
    
    saveSyscfg=rSYSCFG;

    rSYSCFG=SYSCFG_0KB; 		      
    for(i=0x10004000;i<0x10004800;i+=16)    
    {					   
		*((int *)i)=0x0;		   
    }
    rSYSCFG=saveSyscfg; 			    
}

void sys_init()// Interrupt,Port and UART
{
    /* enable interrupt */
	rINTMOD    = 0x0;		// Configura las linas como de tipo IRQ
	rINTCON    = 0x0;	    // Habilita int. vectorizadas y las lineas IRQ y FIQ
    rI_ISPC = 0xffffffff;			/* clear all irq pending */
    rF_ISPC = 0xffffffff;			/* clear all fiq pending */
	rEXTINTPND = 0xf;				// clear EXTINTPND reg
	rINTMSK &= ~(BIT_GLOBAL);		// pone a 0 el bit_global para atender las interrupciones cuyo bit en la máscara sea 0
	Port_Init();					/* Initial 44B0X's I/O port */	

	Delay(0);						/* delay time				*/
	Uart_Init(0,115200);			/* Initial Serial port 1	*/
}
