/**
 * Lab 01 - Task 2 (peripherals.c file)
 * Luke Rogers
 * Jason Colonna
 * 9/15/20
 * Modifications by Nicole Baldy, 11/2021
**/
#include <xc.h> // Generic header for XC16 Compiler
#include "peripherals.h"

#define SCALE 1200L // Found by experimentation (A Poor delay method.)
// In the following, addr = 0 -> access Control, addr = 1 -> access Data
#define BusyLCD() ReadLCD( 0) & 0x80 // D<7> = Busy Flag
#define AddrLCD() ReadLCD( 0) & 0x7F // Not actually used here
#define getLCD() ReadLCD( 1) // Not actually used here.

// In the following, addr = 0 -> access Control, addr = 1 -> access Data
#define putLCD( d) WriteLCD( 1, (d))
#define CmdLCD( c) WriteLCD( 0, (c))
#define HomeLCD() WriteLCD( 0, 2) // See HD44780 instruction set in
#define ClrLCD() WriteLCD( 0, 1) // Table 9.1 of text book

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

void InitPMP( void)
{
    // PMP initialization. See my notes in Sec 13 PMP of Fam. Ref. Manual
    PMCON = 0x8303; // Following Fig. 13-34. Text says 0x83BF (it works) *
    PMMODE = 0x03FF; // Master Mode 1. 8-bit data, long waits.
    PMAEN = 0x0001; // PMA0 enabled
}

void InitLCD( void)
{
    // PMP is in Master Mode 1, simply by writing to PMDIN1 the PMP takes care
    // of the 3 control signals so as to write to the LCD.
    PMADDR = 0; // PMA0 physically connected to RS, 0 select Control register
    PMDIN1 = 0b00111000; // 8-bit, 2 lines, 5X7. See Table 9.1 of text Function set
    msDelay(1); // 1ms > 40us
    PMDIN1 = 0b00001100; // ON, cursor off, blink off
    msDelay(1); // 1ms > 40us
    PMDIN1 = 0b00000001; // clear display
    msDelay(2); // 2ms > 1.64ms
    PMDIN1 = 0b00000110; // increment cursor, no shift
    msDelay(2); // 2ms > 1.64ms
} // InitLCD

char ReadLCD( int addr)
{
    // As for dummy read, see 13.4.2, the first read has previous value in PMDIN1
    int dummy;
    while( PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr; // select the command address
    dummy = PMDIN1; // initiate a read cycle, dummy
    while( PMMODEbits.BUSY); // wait for PMP to be available
    return( PMDIN1); // read the status register
} // ReadLCD

void WriteLCD( int addr, char c)
{
    while (BusyLCD());
    while( PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr;
    PMDIN1 = c;
} // WriteLCD

void putsLCD( const char *s)
{
    while( *s) putLCD( *s++); // See paragraph starting at bottom, pg. 87 text
} //putsLCD

void SetCursorAtLine(int i)
{
    int k;
    if (i == 1)
        CmdLCD(0x80); // Set DDRAM (i.e., LCD) address to upper left (0x80 | 0x00)
    else if(i == 2)
        CmdLCD(0xC0); // Set DDRAM (i.e., LCD) address to lower left (0x80 | 0x40)
    else
    {
        TRISA = 0x00; // Set PORTA<7:0> for output.
        for (k = 1; k < 20; k++) // Flash all 7 LED's @ 5Hz. for 4 seconds.
        {
            PORTA = 0xFF;
            msDelay(100); // 100 ms for ON then OFF yields 5Hz
            PORTA = 0x00;
            msDelay(100);
        }
    }
}

void I2Cinit(int BRG) 
{
    I2C1BRG = BRG; //See PIC24FJ128GA010 data sheet, Table 16.1 pg. 139 
    while (I2C1STATbits.P); // Check buss idle, see 5.1 of I2C document. 
    // It works, not sure its needed. 
    I2C1CONbits.A10M = 0; // 7-bit address mode (Added 8-14-17) 
    I2C1CONbits.I2CEN = 1; // enable module 
}

void I2CStart(void) 
{
    us_delay(10); // delay to be safe 
    I2C1CONbits.SEN = 1; // Initiate Start condition see pg. 21 of I2C man. DS70000195F 
    while (I2C1CONbits.SEN); // wait for Start condition complete See sec. 5.1 
    us_delay(10); // delay to be safe 
}

void I2CStop(void) 
{
    us_delay(20); // delay to be safe 

    I2C1CONbits.PEN = 1; // see 5.5 pg. 27 of Microchip I2C manual DS70000195F 
    while (I2C1CONbits.PEN); // This is at hardware level, & I suspect fast.
    us_delay(20); // delay to be safe 

}

void I2Csendbyte(char data) 
{
    while (I2C1STATbits.TBF); //wait if buffer is full 
    I2C1TRN = data; // pass data to transmission register 
    us_delay(10); // delay to be safe 
}

char I2Cgetbyte(void) 
{
    I2C1CONbits.RCEN = 1; // Set RCEN, Enables I2C Receive mode 
    us_delay(10); // Configure timer and give time to receive
    TMR1 = 0;
    while (!I2C1STATbits.RBF && TMR1 < 100 * 2); //wait for byte to shift into I2C1RCV register, max 100 us.
    // TODO(NEB): Fix this - for some reason, the I2C bus freezes if there is an ack. 
    //    I2C1CONbits.ACKEN = I2C1STATbits.RBF; // Master sends Acknowledge 
    us_delay(10); // delay to be safe 
        
    return (I2C1RCV);
}

void us_delay(int n)
{
    T1CON = 0x8010; // Timer 1 on, Prescale 8:1 
    TMR1 = 0;
    while (TMR1 < n * 2); // (1/(16MHz/8))*2 = 1us, i is multiplier for ius. 
}
