/* 
 * File:  MG996R Servo control 
 * Author: Nicole Baldy
 * Comments: Control a servo lock (Implementation)
 */

#include <xc.h> // include processor files - each processor file is guarded.  
#include "peripherals.h"
#include "HCSR04.h"

void InitUSensor(void)
{
    // Setup HC-SR04
    ECHO_TRIS = 1; // Echo is the US output
    TRIG_TRIS = 0; // Trigger is the US input
}

double GetDistanceCm(void)
{
    // Trigger HCSR04    
    TRIG_PIN = 0;
    us_delay(5);
    TRIG_PIN = 1;
    us_delay(10);
    TRIG_PIN = 0;
    
    unsigned int n = 0;

    // TODO: Ensure a non-timeout condition and make this timer configuration safe. 
    while(!ECHO_PIN && n < 65535){ Nop(); n++; }
    
    TMR1 = 0;
    n = 0;
    while(ECHO_PIN && n < 65535){ Nop(); n++; }
    
    return TMR1 / 58;
}
