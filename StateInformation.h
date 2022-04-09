/* 
 * File:   Smart UV State Machine
 * Author: Nicole Baldy
 * Comments: State Machine Interface Definition
 *  Used in public interface of states, faults, and external (user-controlled) transition events
 */

#ifndef STATE_INFORMATION_H
#define	STATE_INFORMATION_H

#define STR_CODE_WIDTH 6 /* 5 chars + Null*/

#include <xc.h> // include processor files - each processor file is guarded.  

/* strcpy */
#include <stdio.h>
#include <string.h>

typedef enum StateName
{
    STATE_UNKNOWN,
    STATE_INITIALIZATION,
    STATE_WAIT_FOR_OBJECT,
    STATE_VERIFY_CHAMBER_READY,
    STATE_ACTIVE_CYCLE,
    STATE_WAIT_FOR_RELEASE,
    STATE_FAULT,
    STATE_DOOR_OPENING,
    STATE_RUNNING // Parent State Only
} StateName;

typedef enum FaultName
{
    FAULT_UNKNOWN,
    NO_FAULT,
    FAULT_ESTOP,
    FAULT_DOOR_OPEN, //  FAULT or WARN
    FAULT_TIMER_ERROR,
    FAULT_INVALID_STATE,
    FAULT_SENSOR_ERROR,
    FAULT_MOTOR_JAMMED, 
    WARN_EARLY_END_DETECTED,
    WARN_RELEASE_TIMEOUT,
    WARN_PERSON_DETECTED
} FaultName;

const char* getStateStr(StateName state);
const char* getFaultStr(FaultName fault);

#endif	/* STATE_INFORMATION_H */

