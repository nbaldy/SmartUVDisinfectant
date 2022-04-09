/* 
 * File:   Smart UV State Machine
 * Author: Nicole Baldy
 * Comments: State Machine Definition
 */

#ifndef SMART_UV_STATE_MACHINE_H
#define	SMART_UV_STATE_MACHINE_H

#include <xc.h> // include processor files - each processor file is guarded.  

/* strcpy */
#include <stdio.h>
#include <string.h>

#include "StateInformation.h"

#define TRUE 1
#define FALSE 0
#define bool unsigned int

typedef struct State
{
    // Parent state or unknown if None. Parent executes first.
    enum StateName state_name;
    unsigned char display;
    enum FaultName active_fault;
    bool cycle_ok;
    int seconds_remaining;
    // TODO(NEB): Store last known sensor status, Estop status, etc here.
} State;

/* Takes current state, does all required actions, 
 * and then returns the state to process next cycle.
 * "Public Interface", handles calls to all functions below*/
void processCurrentState(State* current_state);

State InitStateMachine();

// Treat below functions as private.
void Initialization(State* state);
void OpenDoor(State* state);
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

// Perform standard state transition actions
// Clear LCD 2nd Row
// Clear U2 Commands
void Transition(State *state, StateName new_state);

// TODO(NEB): Temporary for state proof.
enum Buttons
{
    BUTTON_READY_FOR_NEXT=0x0001, // S4 generally moves forward/input recieved as per "usual use case"
    BUTTON_FAULT = 0x0004, // S6 injects a fault (skip S5)
    BUTTON_CLEAR_FAULT = 0x0008 // S3 clears a fault.
};

#endif	/* SMART_UV_STATE_MACHINE_H */

