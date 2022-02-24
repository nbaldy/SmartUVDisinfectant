/* 
 * File:  MG996R Servo control 
 * Author: Nicole Baldy
 * Comments: Control a servo lock
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MG996R_H
#define	MG996R_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define SERVO_PIN PORTGbits.RG15
#define SERVO_TRIS TRISGbits.TRISG15

// From the MG996R Datasheet https://www.electronicoscaldas.com/datasheet/MG996R_Tower-Pro.pdf:
#define SERVO_PWM_PERIOD_US 20*1000
#define SERVO_LO_DC_US 500 // Determined experimentally to differ from 1 on the datasheet
#define SERVO_HI_DC_US 2400 // Determined experimentally to differ from 2 on the datasheet
#define SERVO_LO_ANGLE_DEG 0 
#define SERVO_HI_ANGLE_DEG 180

#define SERVO_DC_SLOPE (SERVO_HI_DC_US - SERVO_LO_DC_US) / (SERVO_HI_ANGLE_DEG - SERVO_LO_ANGLE_DEG)

// Get servo ready for use
void InitServo();

// Command the servo to an angular (in degrees) position
void ServoGoToPosition(int position, int duration_ms);

#endif	/* MG996R_H */

