#include "StateInformation.h"

/**
 * StateeInformation.c
 * State/Fault to String implementations
**/

const char* getStateStr(StateName state)
{
    switch(state)
    {
    case STATE_UNKNOWN:
		return "s:UNK\0";
    case STATE_INITIALIZATION:
		return "s:INT\0";
    case STATE_WAIT_FOR_OBJECT:
		return "s:WOB\0";
    case STATE_VERIFY_CHAMBER_READY:
		return "s:CRY\0";
    case STATE_ACTIVE_CYCLE:
		return "s:ACE\0";
    case STATE_WAIT_FOR_RELEASE:
		return "s:WRL\0";
    case STATE_FAULT:
		return "s:FLT\0";
    case STATE_DOOR_OPENING:
		return "s:ODR\0";
    };
}
const char* getFaultStr(FaultName fault)
{
    switch(fault) 
    {
    case FAULT_UNKNOWN:
		return "f:UNK\0";
    case FAULT_ESTOP:
		return "f:ESP";
    case FAULT_DOOR_OPEN:
		return "f:DON\0";
    case FAULT_TIMER_ERROR:
		return "f:TMO\0";
    case FAULT_INVALID_STATE:
		return "f:BST\0";
    case FAULT_SENSOR_ERROR:
		return "f:SNS\0";
    case FAULT_MOTOR_JAMMED:
        return "f:MOJ\0";
    case WARN_EARLY_END_DETECTED:
        return "f:EED\0";
    case WARN_RELEASE_TIMEOUT:
        return "f:RLT\0";
    case WARN_PERSON_DETECTED:
        return "f:PDT\0";
    case NO_FAULT:
        return "f:CLR\0";
    default:
        return "\0";
    };
}
