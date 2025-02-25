#include <stdio.h>
/* GPIOE */
#define GPIO_E 0x40021000
#define GPIO_E_MODER ((volatile unsigned int*) GPIO_E)
#define GPIO_E_MODER_LOW  (GPIOE)
#define GPIO_E_MODER_HIGH (GPIOE +  2)
#define GPIO_E_ODR ((volatile unsigned short*) (GPIO_E+0x14))
#define GPIO_E_IDR ((volatile unsigned short*) (GPIO_E+0x10))
#define GPIO_E_Otyper ((volatile unsigned short*) (GPIO_E+0x04))
#define GPIO_E_Ospeedr ((volatile unsigned int*) (GPIO_E+0x08))
#define GPIO_E_Pupdr ((volatile unsigned int*) (GPIO_E+0x0C))

#define GPIO_E_IdrLow ((volatile unsigned char*) GPIO_E+0x10)
#define GPIO_E_IDrHigh ((volatile unsigned char*) GPIO_E+0x10+1)
#define GPIO_E_ODRLow ((volatile unsigned char*) GPIO_E+0x14)
#define GPIO_E_ODRHigh ((volatile unsigned char*) (GPIO_E+0x14+1))



#define STK 0xE000E010
#define STK_CTRL ((volatile unsigned int*) STK)
#define STK_LOAD ((volatile unsigned int*) (STK+0x4))
#define STK_VAL ((volatile unsigned int*) (STK+0x8))


#define PERIOD 100
volatile unsigned char currently_pressed_key = 0xFF; 
#define SYSTCG_EXTICR1 0x40013800 + 8
#define SYSTCG_EXTICR3 0x40013800 + 0x10

// I den här funktionen skall lägre byten av Port E förberedas för att lägga
// ut en signal på pinne 0. Den skall bara kallas en gång. 
void InitSquareWave()
{
    
    *GPIO_E_MODER &= ~(0x3); // keep the rest of the bits and set the first pine to output
    *GPIO_E_MODER |= 0x1;     
    
    
}

// När man kallat den här funktionen skall en fyrkantsvåg med given period 
// (i mikrosekunder) läggas ut på pinne 0, Port E. Perioden skall kunna 
// vara upp till 99000 mikrosekunder.

void delay_mikro(unsigned int us){
    #ifdef SIMULATOR
        us = us / 1000;
        us++;
    #else
        for(unsigned int i = 0; i <= us; i++){
            delay_250ns();
            delay_250ns();
            delay_250ns();
            delay_250ns();
        }
    #endif
}

void StartSquarewave(unsigned int period_in_us)
{
    while(1){
        *GPIO_E_ODRLow = 0x1;
        delay_mikro(period_in_us);
        *GPIO_E_ODRLow = 0x0;
        delay_mikro(period_in_us);
    }
  
}

// Den här funktionen skall avbryta fyrkantsvågen. 
void StopSquareWave()
{
    
    *STK_CTRL = 0; 
}


void delay_250ns(void){
    *STK_CTRL = 0;
    *STK_LOAD = 0x00000029; // 42Hz = 250ns, 42 (dec) = 29 (hex)
    *STK_VAL = 0;
    *STK_CTRL = 5;

    while((*STK_CTRL & 0x00010000)== 0);
    *STK_CTRL = 0;

}
int read_column()
{
    
    unsigned char c = *GPIO_E_IDrHigh;

    if ( c & 0b1000 )
        return 4;

    if ( c & 0b0100 )
        return 3;

    if ( c & 0b0010 )
        return 2;

    if ( c & 0b0001 )
        return 1;

    return 0;
}
void keyboardHandler(){
    *((unsigned int*) SYSTCG_EXTICR3) &=0xFFFF;

}

void kbdActivate( unsigned int row )
{
	switch( row )
	{
		case 1: *GPIO_E_ODRHigh = 0x10;     break;
        case 2: *GPIO_E_ODRHigh = 0x20;     break;
        case 3: *GPIO_E_ODRHigh = 0x40;     break;
        case 4: *GPIO_E_ODRHigh = 0x80;     break;
        case 0: *GPIO_E_ODRHigh = 0x00;     break;
	}
}

// Läs en rad och returnera vilken kolumn som är ett 
// (antar endast en tangent nedtryckt)
int kbdGetCol ( void )
{
	unsigned short c;
	c = *GPIO_E_IDrHigh;
 	if ( c & 0x8 )	return 4;
	if ( c & 0x4 ) 	return 3;
	if ( c & 0x2 ) 	return 2;
	if ( c & 0x1 ) 	return 1;
	return 0;
}

unsigned char keyb(void) 
{
	unsigned char key[] = { '1', '2', '3', 'A',
							'4', '5', '6', 'B',
							'7', '8', '9', 'C',
							'E', '0', 'F', 'D' };
	for (int row=1; row <=4 ; row++) {
		kbdActivate(row);
		delay_250ns();		
		int col = kbdGetCol();
		if(col)	{ 
			return key [4*(row-1)+(col-1)];
		}
	}
	*GPIO_E_ODRHigh = 0;
	return 0xFF;
}

void InitKeyboard(){
   // från labb 2 här
    
    // b15-b12 used for output to rows
	// b11-b8  used for input from columns
	*GPIO_E_MODER = 0x55005555;	
    // Pinnarna som läses från tangentbordet är spänningssatta om
	// nedtryckta och flytande annars, så behöver Pull Down
	*GPIO_E_Pupdr = 0x00AA0000;		
	// Pinnarna som väljer rad skall vara spänningssatta (Push/Pull)
    *GPIO_E_Otyper= 0x00000000;	 


    *GPIO_E_Ospeedr = 0x00000000;
	*GPIO_E_ODRLow = 0;
	*GPIO_E_ODRHigh = 0;

    int c; 
			do { 
				c = keyb(); 
			} while(c == 0xFF);
    
    
    //vecktor shit här 

    *((unsigned int*) SYSTCG_EXTICR3) &=0xFFFF; // nollställa 
    *((unsigned int*) SYSTCG_EXTICR3) |=0x4444; // 0100 to EXTI 8-9

   // 
   *((unsigned int*) 0x40013C00) |=8;
   *((unsigned int*) 0x40013C0C) |=8;
   *((unsigned int*) 0x40013C0C) &= ~8;



    
}



void main()
{
    InitSquareWave();
    InitKeyboard();
    StartSquarewave(PERIOD); 
    while(currently_pressed_key != 1) { /* Gör ingenting förrän man trycker på knapp '1'*/ }
    StopSquareWave(); 
    while(currently_pressed_key != 5) { /* Gör ingenting förrän man trycker på knapp '5'*/ }
    StartSquarewave(PERIOD * 2); 
    while(currently_pressed_key != 9) { /* Gör ingenting förrän man trycker på knapp '9'*/ }
    StopSquareWave(); 
}