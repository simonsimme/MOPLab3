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


//Den här funktionen skall konfigurera övre byten av Port E och aktivera ALLA rader på keyboarden.
// Interrupts skall initieras så att en interrupt handler kallas när någon knapp trycks ner. 
// Interrupthandlern skall uppdatera "currently_pressed_key"
void InitKeyboard(){

    /* Konfigurera pinnar 8-15 som ingångar: rensa motsvarande MODER-bitar */
    *GPIO_E_MODER &= 0x0000FFFF;


    // *(volatile ulong*)0x40023830 = 0x18;
    // *(volatile uint*)0x40020C08 = 0x55555555;

    // *(volatile ushort*) GPIO_E_MODER_LOW  = 0x5555;
    // *(volatile ushort*) GPIO_E_MODER_HIGH = 0x5500;
    // *(volatile ushort*) GPIO_E_Otyper    &= 0x00FF;
    // *(volatile uint*)   GPIO_E_Pupdr     &= 0x0000FFFF;
    // *(volatile uint*)   GPIO_E_Pupdr     |= 0x00AA0000;
    // printf(read_column());
    

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