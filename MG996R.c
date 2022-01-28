// Implementation file for servo

#include "MG996R.h"
#include "peripherals.h"

#include <xc.h> // include processor files - each processor file is guarded.  

void InitServo()
{
    SERVO_TRIS = 0; // Output
}

void ServoGoToPosition(int position, int duration_ms)
{
    if(position < SERVO_LO_ANGLE_DEG)
    {
        // ERROR: Servo commanded to invalid angle, cap. 
        position = SERVO_LO_ANGLE_DEG;
    }
    else if(position > SERVO_HI_ANGLE_DEG)
    {
        // ERROR: Servo commanded to invalid angle, cap. 
        position = SERVO_HI_ANGLE_DEG;        
    }
    
    int hi_duration_us = (SERVO_LO_DC_US + 
        SERVO_DC_SLOPE * (position - SERVO_LO_ANGLE_DEG));
    int lo_duration_us = SERVO_PWM_PERIOD_US - hi_duration_us;

    int duration_passed;
    for (duration_passed = 0; 
            duration_passed < duration_ms; 
            duration_passed += SERVO_PWM_PERIOD_US/1000)
    {
        SERVO_PIN = 1;
        us_delay(hi_duration_us);
        SERVO_PIN = 0;
        us_delay(lo_duration_us);
    }
}