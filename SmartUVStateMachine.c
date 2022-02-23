// Implementation file for state machine

#include "SmartUVStateMachine.h"
#include "peripherals.h"
#include "AMG88.h" // IR Grid-eye
#include "timer.h"
#include "MG996R.h"
#include "HCSR04.h"
#include   <stdio.h>
#include   <stdlib.h>
#include   <string.h>
#include "bluetooth.h"

#define LED_PIN PORTGbits.RG14
#define LED_TRIS TRISGbits.TRISG14
#define DOOR_PIN PORTGbits.RG13
#define DOOR_TRIS TRISGbits.TRISG13

#define LED_ON 0
#define LED_OFF 1

#define MAX_DIST 25 // Should be 61 but reduced for cardboard demo

StateNameStr getStateNameStr(enum StateName state_enumeration)
{
    StateNameStr str_repr;

    switch(state_enumeration)
    {
        case STATE_INITIALIZATION:
           strcpy(str_repr.str, "INIT            ");
           break;
        case STATE_DOOR_OPENING:
           strcpy(str_repr.str, "OPEN DOOR       ");
           break;
        case STATE_WAIT_FOR_OBJECT:
           strcpy(str_repr.str, "WAIT FOR OBJ    ");
           break;
        case STATE_VERIFY_CHAMBER_READY:
           strcpy(str_repr.str, "CHECK READY     ");
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
    InitUSensor();
    I2Cinit(157);
    InitServo();
    InitU2();

    // Use P97 = RG13 for door input
    DOOR_TRIS = 1;
    // Use P95 = RG14 for LED output
    LED_TRIS = 0;
    LED_PIN = LED_OFF;

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
        case STATE_DOOR_OPENING:
        {
            OpenDoor(current_state);
            break;
        }
        case STATE_UNKNOWN:
        default:
        {
            // UNSUPPORTED CHILD STATE
            SetFault(current_state);
            current_state->active_fault = FAULT_INVALID_STATE;
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
    int command = getCommand();
    if (getButton(BUTTON_READY_FOR_NEXT) || command)
    {
        // NOTE(NEB): For now, consider "initialized" when button pressed.
        // Event: Init Complete
        state->state_name = STATE_DOOR_OPENING;
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

void OpenDoor(State* state)
{
    Running(state); // Parent State

    // Open door
    ServoGoToPosition(0, 400);

    if (0 == DOOR_PIN) // P97 = RG13 should be used for door input
    {
        // Delay extra 100 ms to ensure door opens all the way 
        msDelay(500);

        // Event: Door Opened
        state->state_name = STATE_WAIT_FOR_OBJECT;
        ServoGoToPosition(180, 1000);
        return;
    }

    // TODO(NEB): Wait for door closed sense
    state->display |= 0x82;
}

void VerifyChamberReady(State* state)
{
    bReadTempFromGridEYE();
    msDelay(10); // Give time to Read Everything
    Running(state); // Parent State

    int num_pxls_body_temp = numPixelsInRange(27*256, 40*256); // between 27 and 40 C for now
    int is_person_detected = (num_pxls_body_temp >= 4); /* Experimental: 4+ pixels is a very good indicator of a person */

    double dist_cm = GetDistanceCm();
    int is_open_door_detected = (dist_cm > MAX_DIST); /* about 2 ft */

    SetCursorAtLine(2);
    
    int command = getCommand();
//    int command = getButton(BUTTON_READY_FOR_NEXT)? 1: 0;
    int is_start = (command == 1);
    
    char str[16];
    sprintf(str, "got%c %d, isS%d", (char)command, command, is_start);
//    putsLCD(str);
    msDelay(500);
    if (is_start && (!is_person_detected && !is_open_door_detected))
    {
        // Event: Chamber Ready and got UI command [No Person AND Door closed AND UI Cmd]
        state->state_name = STATE_ACTIVE_CYCLE;
        ConfigureLongTimer(5*60);
        StartLongTimer();
        return;
    }

    // Something is not ready, print warnings
    // TODO[NEB]: Send to UI
    char info_str[12];
    char err_str[16] = "";
    SetCursorAtLine(2);
    sprintf(info_str, "%dpx %1.1fcm", num_pxls_body_temp, dist_cm);
    if (is_person_detected)
    {
        strcat(err_str, "!!");
    }
    // String is [!!]? px cm [!!]?
    strcat(err_str, info_str);

    if (is_open_door_detected)
    {
        strcat(err_str, "!!");
    }
    putsLCD(err_str);

    if(0 == DOOR_PIN)
    {
        // Door opened verification, wait for closed again
        state->state_name = STATE_WAIT_FOR_OBJECT;
        return;
    }
 
    // TODO(NEB): Wait for door closed sense
    state->display |= 0x03;
}

void ActiveCycle(State* state)
{
    Running(state); // Parent State

    if(0 == DOOR_PIN)
    {
        // Door opened during active cycle, fault
        LED_PIN = LED_OFF;
        state->active_fault = FAULT_DOOR_OPEN;
        SetFault(state);
        return;
    }

    enum LongTimerStatus tmr_status = CheckTimerStatus();
    if (STATUS_ERR == tmr_status)
    {
        LED_PIN = LED_OFF;
        state->active_fault = FAULT_TIMER_ERROR;
        SetFault(state);
        return;
    }
    int command = getCommand();

    if ( (STATUS_DONE == tmr_status) || command == 1)
    {
        // NOTE(NEB): For now, consider "stopped" when button pressed or timer expired.
        // Event: Stop Cmd Recieved
        state->state_name = STATE_WAIT_FOR_RELEASE;
        LED_PIN = LED_OFF;
        return;
    }

    LED_PIN = LED_ON;

    // TODO(NEB): Wait for stop cmd from wireless
    state->display = (GetSecondsElapsed() & 0x7f);
}

void WaitForRelease(State* state)
{
    // TODO(NEB): Unlock when get command from UI.
    Running(state); // Parent State
    int command = getCommand();

    if (getButton(BUTTON_READY_FOR_NEXT) || command) // Move to next state when get user command
    {
        // Event: Unlock Cmd Recieved
        state->state_name = STATE_DOOR_OPENING;
        return;
    }

    // TODO(NEB): Wait for unlock cmd
    state->display |= 0x06;
}

void Fault(State* state)
{
    int command = getCommand();
    if (getButton(BUTTON_CLEAR_FAULT) || command)
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
    LED_PIN = LED_OFF;
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
        case FAULT_TIMER_ERROR:
            putsLCD("TIMER ERR    ");
            break;
        case FAULT_INVALID_STATE:
            putsLCD("INVALID STATE");
            break;
        default:
        case FAULT_UNKNOWN:
            putsLCD("Unknown Fault");
            break;
    }
}