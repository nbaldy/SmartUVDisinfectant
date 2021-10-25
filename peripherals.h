/**
 * Lab 01 - Task 2 (peripherals.h file)
 * Luke Rogers
 * Jason Colonna
 * 9/15/20
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

#ifdef	__cplusplus
}
#endif

#endif	/* PERIPHERALS_H */

