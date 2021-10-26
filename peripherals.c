/**
 * Lab 01 - Task 2 (peripherals.c file)
 * Luke Rogers
 * Jason Colonna
 * 9/15/20
**/
#include <xc.h> // Generic header for XC16 Compiler
#define SCALE 1200L // Found by experimentation (A Poor delay method.)

void initPortA(void)
{
    PORTA = 0x0000; //clear port A
    TRISA = 0xFF80; // set PORTA <6:0> to output
}
void initButtons(unsigned int mask)
{
    if(mask&0x0008) TRISDbits.TRISD6 = 1; // S3 TRISA
    if(mask&0x0004) TRISDbits.TRISD7 = 1; // S6 TRISA
    if(mask&0x0002) TRISAbits.TRISA7 = 1; //S5 TRISA
    if(mask&0x0001) TRISDbits.TRISD13 = 1; // S4 TRISA
}

unsigned int getButton(unsigned int mask) {
    unsigned int button;
    initButtons(0x000F);
    switch (mask) {
        case 0x0008: button = !PORTDbits.RD6; // S3
            break;
        case 0x0004: button = !PORTDbits.RD7; // S6
            break;
        case 0x0002: TRISAbits.TRISA7 = 1; // S5
            button = !PORTAbits.RA7;
            break;
        case 0x0001: button = !PORTDbits.RD13; // S4
            break;
        default:
            button = 0;
    }
    return (button);
}

void setPortA(unsigned int display)
{
    TRISAbits.TRISA7 = 0;
    PORTA = 0x00FF&display; // transfer to PORTA (LED?s)
}

void msDelay(unsigned int ms)
{
    unsigned long i;
    for(i=(unsigned long)(ms)*SCALE; i>0; i-=1)
        Nop();
}

