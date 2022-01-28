/**
 * Last Modified by Nicole Baldy, 1/22
 * 
 * HC-SR04 is an ultrasonic distance sensor.
**/

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HC_SR04_H
#define	HC_SR04_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define ECHO_PIN PORTGbits.RG0
#define TRIG_PIN PORTGbits.RG1
#define ECHO_TRIS TRISGbits.TRISG0
#define TRIG_TRIS TRISGbits.TRISG1

void InitUSensor(void);
double GetDistanceCm(void);

#endif	/* HC_SR04_H */

