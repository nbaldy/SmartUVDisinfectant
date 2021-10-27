/**
 * Lab 01 - Task 2 (peripherals.h file)
 * Luke Rogers
 * Jason Colonna
 * 9/15/20
 * Modified by Nicole Baldy, 10/21
**/

#ifndef PERIPHERALS_H
#define	PERIPHERALS_H


#ifdef	__cplusplus
extern "C" {
#endif

void initPortA(void);
void initButtons(unsigned int mask);
void setPortA(unsigned int display);
void msDelay(unsigned int ms);
unsigned int getButton(unsigned int mask);

// LCD
void InitPMP(void);
void InitLCD(void);
char ReadLCD(int addr);
void WriteLCD(int addr, char c);
void putsLCD(char *s);
void SetCursorAtLine(int i);


#ifdef	__cplusplus
}
#endif

#endif	/* PERIPHERALS_H */

