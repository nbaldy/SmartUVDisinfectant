// Implementation file for state machine

#include "SmartUVStateMachine.h"
#include "peripherals.h"
#include "AMG88.h" // IR Grid-eye

#include   <stdio.h>
#include   <stdlib.h>
#include   <string.h>

#define LED_PIN PORTGbits.RG14
#define LED_TRIS TRISGbits.TRISG14
#define DOOR_PIN PORTGbits.RG13
#define DOOR_TRIS TRISGbits.TRISG13

StateNameStr getStateNameStr(enum StateName state_enumeration)
{
    StateNameStr str_repr;

    switch(state_enumeration)
    {
        case STATE_INITIALIZATION:
           strcpy(str_repr.str, "INIT            ");
           break;
        case STATE_WAIT_FOR_OBJECT:
           strcpy(str_repr.str, "WAIT FOR OBJ    ");
           break;
        case STATE_VERIFY_CHAMBER_READY:
           strcpy(str_repr.str, "CHECK READY     ");
           break;
        case STATE_WAIT_FOR_CYCLE_START:
           strcpy(str_repr.str, "WAIT FOR START  ");
           break;
        case STATE_ACTIVE_CYCLE:
           strcpy(str_repr.str, "CYCLE ACTIVE    ");
           break;
        case STATE_WAIT_FOR_RELEASE:
           strcpy(str_repr.str, "WAIT FOR RELEASE");
           break;
        case STATE_FAULT:
           strcpy(str_repr.str, "FAULT           ");
           break;
        default:
           strcpy(str_repr.str, "UNKNOWN STATE   ");
           break;
    }
    
    return str_repr;
}


struct State InitStateMachine()
{
    State new_sm;
    new_sm.display = 0x00;
    new_sm.state_name = STATE_UNKNOWN;
    initPortA();
    initButtons(0x000F); // All buttons as inputs: 0xF
    msDelay(100); // Give time to start up
    InitPMP();
    InitLCD();
    I2Cinit(157);
    
    // Use P97 = RG13 for door input
    DOOR_TRIS = 1;
    // Use P95 = RG14 for LED output
    LED_TRIS = 0;
    LED_PIN = 0;

    new_sm.active_fault = FAULT_UNKNOWN;
    new_sm.state_name = STATE_INITIALIZATION;
    return new_sm;
}

void processCurrentState(State* current_state)
{
    // Rebuilt display with each tick (TODO - reconsider this.)
    current_state->display = 0x00;
    StateNameStr state_str = getStateNameStr(current_state->state_name);

    // Print State Name at the top left corner.
    SetCursorAtLine(1);
    putsLCD((char *)state_str.str);

    switch(current_state->state_name)
    {
        case STATE_INITIALIZATION:
        {
            Initialization(current_state);
            break;
        }
        case STATE_WAIT_FOR_OBJECT:
        {
            WaitForObject(current_state);
            break;
        }
        case STATE_VERIFY_CHAMBER_READY:
        {
            VerifyChamberReady(current_state);
            break;
        }
        case STATE_WAIT_FOR_CYCLE_START:
        {
            WaitForCycleClart(current_state);
            break;
        }
        case STATE_ACTIVE_CYCLE:
        {
            ActiveCycle(current_state);
            break;
        }
        case STATE_WAIT_FOR_RELEASE:
        {
            WaitForRelease(current_state);
            break;
        }
        case STATE_FAULT:
        {
            Fault(current_state);
            break;
        }
        case STATE_UNKNOWN:
        default:
        {
            // UNSUPPORTED CHILD STATE
            SetFault(current_state);
            return;
        }
    }
    
    // TODO(NEB): Temporary way of showing state.
    setPortA(current_state->display);
}

// "Private"


void Initialization(State* state)
{
    Running(state); // Parent State
    if (getButton(BUTTON_READY_FOR_NEXT))
    {
        // NOTE(NEB): For now, consider "initialized" when button pressed.
        // Event: Init Complete
        state->state_name = STATE_WAIT_FOR_OBJECT;
        // Clear any initialization faults
        state->active_fault = NO_FAULT;
        return;
    }
    
    // TODO(NEB): Initialize all sensors.
    state->display |= 0x01;
}

void WaitForObject(State* state)
{
    Running(state); // Parent State
    if (1 == DOOR_PIN) // P97 = RG13 should be used for door input
    {
        // Event: Door Closed
        state->state_name = STATE_VERIFY_CHAMBER_READY;
        return;
    }
    
    // TODO(NEB): Wait for door closed sense
    state->display |= 0x02;
}

void VerifyChamberReady(State* state)
{
    bReadTempFromGridEYE();
    msDelay(10); // Give time to Read Everything
    Running(state); // Parent State
    
    short max_pxl = maxPixel(); // 256 * Temp_In_C
    
    SetCursorAtLine(2);
    char str[16];
    double max_C = (double) max_pxl / (256); 
    int num_pxls_body_temp = numPixelsInRange(27*256, 40*256); // between 27 and 40 C for now

    sprintf(str, "%4.2f C; r=%d", max_C, num_pxls_body_temp);

    putsLCD(str);

    if(0 == DOOR_PIN)
    {
        // Door opened verification, wait for closed again
        state->state_name = STATE_WAIT_FOR_OBJECT;
        return;
    }
    if (getButton(BUTTON_READY_FOR_NEXT) || 3 < num_pxls_body_temp) // More than 3 pixels in this range
    {
        // NOTE(NEB): For now, consider "ready" when button pressed.
        // Event: Chamber Ready
        state->state_name = STATE_WAIT_FOR_CYCLE_START;
        return;
    }
    
    // TODO(NEB): Wait for door closed sense
    state->display |= 0x03;
}

void WaitForCycleClart(State* state)
{
    Running(state); // Parent State

    if(0 == DOOR_PIN)
    {
        // Door opened during wait for cycle, fault
        state->active_fault = FAULT_DOOR_OPEN;
        SetFault(state);
        return;
    }

    if (getButton(BUTTON_READY_FOR_NEXT))
    {
        // NOTE(NEB): For now, consider "started" when button pressed.
        // Event: Start Cmd Recieved
        state->state_name = STATE_ACTIVE_CYCLE;
        return;
    }
    
    // TODO(NEB): Wait for start cmd from wireless
    state->display |= 0x04;
}

void ActiveCycle(State* state)
{
    Running(state); // Parent State
    if(0 == DOOR_PIN)
    {
        // Door opened during active cycle, fault
        LED_PIN = 0;
        state->active_fault = FAULT_DOOR_OPEN;
        SetFault(state);
        return;
    }

    if (getButton(BUTTON_READY_FOR_NEXT))
    {
        // NOTE(NEB): For now, consider "stopped" when button pressed.
        // Event: EITHER Stop Cmd Recieved OR Timer Expired
        state->state_name = STATE_WAIT_FOR_RELEASE;
        LED_PIN = 0;
        return;
    }
    
    LED_PIN = 1;
    
    // TODO(NEB): Wait for stop cmd from wireless OR timer complete
    state->display |= 0x05;
}

void WaitForRelease(State* state)
{
    // TODO(NEB): Unlock when get command from UI.
    Running(state); // Parent State
    if (0 == DOOR_PIN) // Move to next state when door opens/pull down
    {
        // Event: Unlock Cmd Recieved
        state->state_name = STATE_WAIT_FOR_OBJECT;
        return;
    }
    
    // TODO(NEB): Wait for unlock cmd
    state->display |= 0x06;
}

void Fault(State* state)
{
    if (getButton(BUTTON_CLEAR_FAULT))
    {
        // NOTE(NEB): For now, consider "fault cleared" when button pressed.
        // Event: Fault Cleared
        state->active_fault = NO_FAULT;
        state->state_name = STATE_INITIALIZATION;
        return;
    }

    state->display = 0x7F; // Active Fault
    printFaultState(state->active_fault);
}

// Checks for faults & other "general" parent state needs
void Running(State* state)
{
    if (getButton(BUTTON_FAULT))
    {
        SetFault(state);
        return;
    }

    state->display |= 0x80; // Set 1st LED
    printFaultState(state->active_fault);
}

void SetFault(State *state)
{
    state->state_name = STATE_FAULT;
    state->display = 0x3F; // Unprocessed Fault
    LED_PIN = 0; // Turn LEDs off
    setPortA(state->display);
}

void printFaultState(FaultName fault_name)
{
    // Print Door Status at the bottom left.
    SetCursorAtLine(2);
    switch(fault_name)
    {
        case NO_FAULT:
            putsLCD("--           ");
            break;
        case FAULT_ESTOP:
            putsLCD("ESTOPPED     ");
            break;
        case FAULT_DOOR_OPEN:
            putsLCD("DOOR OPEN    ");
            break; 
        default:
        case FAULT_UNKNOWN:
            putsLCD("Unknown Fault");
            break;
    }
}