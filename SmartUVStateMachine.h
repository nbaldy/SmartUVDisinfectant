/* 
 * File:   Smart UV State Machine
 * Author: Nicole Baldy
 * Comments: State Machine Interface Definition
 * Revision history: Initial Revision
 */

#ifndef SMART_UV_STATE_MACHINE_H
#define	SMART_UV_STATE_MACHINE_H

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
    STATE_WAIT_FOR_CYCLE_START,
    STATE_ACTIVE_CYCLE,
    STATE_WAIT_FOR_RELEASE,
    STATE_FAULT,
    STATE_RUNNING // Parent State Only
} StateName;

typedef enum FaultName
{
    FAULT_UNKNOWN,
    NO_FAULT,
    FAULT_ESTOP,
    FAULT_DOOR_OPEN
} FaultName;

// 8-character representation of state name for display
typedef struct StateNameStr
{
    char str[17]; // 16 char + nullcharacter
} StateNameStr;

StateNameStr getStateNameStr(enum StateName state_enumeration);


typedef struct State
{
    // Parent state or unknown if None. Parent executes first.
    enum StateName state_name;
    unsigned char display;
    enum FaultName active_fault;
    // TODO(NEB): Store last known sensor status, Estop status, etc here.
} State;

/* Takes current state, does all required actions, 
 * and then returns the state to process next cycle.
 * "Public Interface", handles calls to all functions below*/
void processCurrentState(State* current_state);

State InitStateMachine();


// Treat below functions as private.
void Initialization(State* state);
void WaitForObject(State* state);
void VerifyChamberReady(State* state);
void WaitForCycleClart(State* state);
void ActiveCycle(State* state);
void WaitForRelease(State* state);
void Fault(State* state);
// "Parent" States
void Running(State* state);

void SetFault(State* state); // TODO(NEB): Fault Codes, for now just set state.
void printFaultState(FaultName fault_name);

// TODO(NEB): Temporary for state proof.
enum Buttons
{
    BUTTON_READY_FOR_NEXT=0x0001, // S4 generally moves forward/input recieved as per "usual use case"
    BUTTON_FAULT = 0x0004, // S6 injects a fault (skip S5)
    BUTTON_CLEAR_FAULT = 0x0008 // S3 clears a fault.
};


#endif	/* SMART_UV_STATE_MACHINE_H */

