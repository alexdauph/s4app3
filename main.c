/************************************
 * Name :     main.c
 * Author:    Jp Gouin
 * Version :  h2022
 ************************************/

#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include <string.h>

// Since the flag is changed within an interrupt, we need the keyword volatile.
static volatile int Flag_1s = 0;

void LCD_seconde(unsigned int seconde);
extern void pmod_s();
extern long module_s(int x, int y, int z);

void __ISR(_TIMER_1_VECTOR, IPL2AUTO) Timer1ISR(void) 
{  
   Flag_1s = 1;           //    Indique à la boucle principale qu'on doit traiter
   IFS0bits.T1IF = 0;     //    clear interrupt flag
}

#define TMR_TIME    0.001             // x us for each tick

void initialize_timer_interrupt(void) { 
  T1CONbits.TCKPS = 3;                //    256 prescaler value
  T1CONbits.TGATE = 0;                //    not gated input (the default)
  T1CONbits.TCS = 0;                  //    PCBLK input (the default)
  PR1 = (int)(((float)(TMR_TIME * PB_FRQ) / 256) + 0.5);   //set period register, generates one interrupt every 1 ms
                                      //    48 kHz * 1 ms / 256 = 188
  TMR1 = 0;                           //    initialize count to 0
  IPC1bits.T1IP = 2;                  //    INT step 4: priority
  IPC1bits.T1IS = 0;                  //    subpriority
  IFS0bits.T1IF = 0;                  //    clear interrupt flag
  IEC0bits.T1IE = 1;                  //    enable interrupt
  T1CONbits.ON = 1;                   //    turn on Timer5
} 

void main() {
    LCD_Init();
    LED_Init();
    ACL_Init();
    initialize_timer_interrupt();
    PMODS_InitPin(1,1,0,0,0); // initialisation du JB1 (RD9))

    int count = 0;
    long module = 10;
    unsigned char pmodValue = 0;
    unsigned int seconde = 0 ;

    macro_enable_interrupts();
    
    LCD_WriteStringAtPos("Heure : ", 0, 0);
   
    // Main loop
    while(1) {
        if(Flag_1s)                 // Flag d'interruption à chaque 1 ms
        {
            //pmodValue = PMODS_GetValue(1, 1);
            //pmodValue ^= 1;
            //PMODS_SetValue(1, 1, pmodValue);
            pmod_s();
            module = module_s(1, 2, 3);
            Flag_1s = 0;            // Reset the flag to capture the next event
            if (++count >= 1000) 
            {
                count = 0;
                LED_ToggleValue(0);
                LCD_seconde(++seconde);
            }
        }
    }
}

void LCD_seconde(unsigned int seconde) { 
    LCD_WriteIntAtPos(seconde%60, 3, 0, 13, 0);     
    LCD_WriteStringAtPos(":", 0, 13); // affichage des secondes
    LCD_WriteIntAtPos(seconde/60%60, 3, 0, 10, 0);  
    LCD_WriteStringAtPos(":", 0, 10); // affichage des secondes
    LCD_WriteIntAtPos(seconde/3600%24, 3, 0, 7, 0);  
}