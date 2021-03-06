// Implementation file for state machine

#include "SmartUVStateMachine.h"
#include "peripherals.h"
#include "AMG88.h" // IR Grid-eye
#include "timer.h"
#include "Lock.h"
#include "bluetooth.h"
#include "hullSensor.h"
#include "StateInformation.h"

#include "HCSR04.h"
#include   <stdio.h>
#include   <stdlib.h>
#include   <string.h>


#define LED_PIN PORTGbits.RG14
#define LED_TRIS TRISGbits.TRISG14
#define DOOR_PIN PORTGbits.RG13
#define DOOR_TRIS TRISGbits.TRISG13

#define LED_ON 1
#define LED_OFF 0

#define MAX_DIST 25 // cm
#define MAX_SECONDS_BETWEEN_DETECTION 20 // Experimental - 5 magnets

int lastHullDetectSecond = 0;

struct State InitStateMachine()
{
    State new_sm;
    new_sm.display = 0x00;
    new_sm.cycle_ok = FALSE;
    new_sm.state_name = STATE_UNKNOWN;
    initPortA();
    initButtons(0x000F); // All buttons as inputs: 0xF
    msDelay(100); // Give time to start up
    InitPMP();
    InitLCD();
    InitUSensor();
    I2Cinit(157);
    InitLock();
    initHullSensor();

    // Use P97 = RG13 for door input
    DOOR_TRIS = 1;
    // Use P95 = RG14 for LED output
    LED_TRIS = 0;
    LED_PIN = LED_OFF;

    InitU2();
    new_sm.active_fault = FAULT_UNKNOWN;
    Transition(&new_sm, STATE_INITIALIZATION);
    return new_sm;
}

void processCurrentState(State* current_state)
{
    // Rebuilt display with each tick (TODO - reconsider this.)
    current_state->display = 0x00;
    const char* state_str = getStateStr(current_state->state_name);

    // Print State Name at the top left corner.
    SetCursorAtLine(1);
    putsLCD(state_str);

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
    state->active_fault = FAULT_UNKNOWN;

    // Ensure that we do not move forward on a power cycle due to power to buttons weird
    Running(state); // Parent State

    // NOTE(NEB): "Initialized" when sensors give "reasonable" feedback.
    bReadTempFromGridEYE();
    msDelay(10); // Give time to Read Everything

    double dist_cm = GetDistanceCm();
    int is_us_success = (dist_cm > 0);

    int num_pxls_zero = numPixelsInRange(0, 1);
    int is_grid_eye_success = (num_pxls_zero < 64);

    if ((is_grid_eye_success && is_us_success))
    {
        Transition(state, STATE_DOOR_OPENING);
        // Clear any initialization faults
        state->active_fault = NO_FAULT;
        return;
    }
    else
    {
        state->active_fault = FAULT_SENSOR_ERROR;

        char info_str[17];
        SetCursorAtLine(2);
        sprintf(info_str, "%dpx %1.1fcm", num_pxls_zero, dist_cm);
        putsLCD(info_str);
        msDelay(500);
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
        msDelay(200);
        Transition(state, STATE_VERIFY_CHAMBER_READY);
        return;
    }

    if (GetDistanceCm() < MAX_DIST /*cm*/)
    {
        // Door is being closed, unlock so it can be closed completely
        Unlock();
    }
    else
    {
        // Door completely open, let the solenoid be unpowered
        Lock();        
    }


    // TODO(NEB): Wait for door closed sense
    state->display |= 0x02;
}

void OpenDoor(State* state)
{
    Running(state); // Parent State

    // Open door
    Unlock();

    if (0 == DOOR_PIN) // P97 = RG13 should be used for door input
    {
        // Delay extra 100 ms to ensure door opens all the way
        msDelay(500);

        // Event: Door Opened
        Transition(state, STATE_WAIT_FOR_OBJECT);
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
    Lock();

    int num_pxls_body_temp = numPixelsInRange(27*256, 40*256); // between 27 and 40 C for now
    int is_person_detected = (num_pxls_body_temp >= 4); /* Experimental: 4+ pixels is a very good indicator of a person */

    double dist_cm = GetDistanceCm();
    int is_open_door_detected = (dist_cm != -1 && dist_cm > MAX_DIST); /* about 2 ft */

    // Move to active cycle if ready
    if (((getCommand() == START_CMD) || (getButton(BUTTON_READY_FOR_NEXT)))
            && (!is_person_detected && !is_open_door_detected))
    {
        // Event: Chamber Ready and got UI command [No Person AND Door closed AND UI Cmd]
        Transition(state, STATE_ACTIVE_CYCLE);
        resetHullSensor();
        lastHullDetectSecond = 0;
        ConfigureLongTimer(5*60);
        StartLongTimer();
        return;
    }
    // Move to open door if canceled
    if(getCommand() == END_CMD)
    {
        Transition(state, STATE_DOOR_OPENING);
        return;
    }

    // Something is not ready, print warnings
    // TODO[NEB]: Send to UI
    char info_str[12];
    SetCursorAtLine(2);
    sprintf(info_str, "     %d %1.1f", num_pxls_body_temp, dist_cm);
    putsLCD(info_str);
    state->cycle_ok = TRUE;
    state->active_fault = NO_FAULT;

    if (is_person_detected)
    {
        state->active_fault = WARN_PERSON_DETECTED;
        state->cycle_ok = FALSE;
    }

    if (is_open_door_detected)
    {
        state->active_fault = FAULT_DOOR_OPEN;
        state->cycle_ok = FALSE;
    }

    if(!state->cycle_ok)
    {
        // Wait 0.5 second before checking again
        msDelay(500);
    }

    if(0 == DOOR_PIN)
    {
        // Door opened verification, wait for closed again
        Transition(state, STATE_WAIT_FOR_OBJECT);
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

    if ( (STATUS_DONE == tmr_status) || getCommand() == END_CMD)
    {
        // NOTE(NEB): For now, consider "stopped" when button pressed or timer expired.
        // Event: Stop Cmd Recieved
        Transition(state, STATE_WAIT_FOR_RELEASE);
        LED_PIN = LED_OFF;
        return;
    }

    int currentSecond = GetSecondsElapsed();
    if(detect())
        lastHullDetectSecond = currentSecond;

    if ((currentSecond - lastHullDetectSecond) > MAX_SECONDS_BETWEEN_DETECTION)
    {
        SetCursorAtLine(2);
        LED_PIN = LED_OFF;
        state->active_fault = FAULT_MOTOR_JAMMED;
        SetFault(state);
        Transition(state, STATE_WAIT_FOR_RELEASE);
        return;
    }
    char rpm_str[17];
    sprintf(rpm_str, "%3.2frpm %d %d",
                    (double)(10 *getNumDetections())/GetSecondsElapsed(), /*60s / 12 magnets*/
                    currentSecond, lastHullDetectSecond);
    SetCursorAtLine(2);
    putsLCD(rpm_str);

    int seconds_remaining = GetSecondsRemaining();
    if (state->seconds_remaining != seconds_remaining &&
            (seconds_remaining % 30 == 0))
    {
        char str[6];
        sprintf(str, "t:%d", seconds_remaining);
        sendToU2(str, STR_CODE_WIDTH);
        state->seconds_remaining = seconds_remaining;
    }
    LED_PIN = LED_ON;

    // TODO(NEB): Wait for stop cmd from wireless
    state->display = (GetSecondsElapsed() & 0x7f);
}

int timer_configured = FALSE;
void WaitForRelease(State* state)
{
    // TODO(NEB): Unlock when get command from UI.
    Running(state); // Parent State

    if(timer_configured == FALSE)
    {
        ConfigureLongTimer(1*60); /* 1 min timeout*/
        StartLongTimer();
        timer_configured = TRUE;
    }

    if (getButton(BUTTON_READY_FOR_NEXT) || getCommand() == RELEASE_CMD)
    {
        // Event: Unlock Cmd Recieved
        Transition(state, STATE_DOOR_OPENING);
        return;
    }

    if(STATUS_DONE == CheckTimerStatus())
    {
        sendToU2(getFaultStr(WARN_RELEASE_TIMEOUT), STR_CODE_WIDTH);
        state->active_fault = WARN_RELEASE_TIMEOUT;
        Transition(state, STATE_DOOR_OPENING);
    }

    // TODO(NEB): Wait for unlock cmd
    state->display |= 0x06;
}

void Fault(State* state)
{
    if (getButton(BUTTON_CLEAR_FAULT) || getCommand() == CLR_CMD)
    {
        // NOTE(NEB): For now, consider "fault cleared" when button pressed.
        // Event: Fault Cleared
        state->active_fault = NO_FAULT;
        Transition(state, STATE_INITIALIZATION);
        return;
    }

    LED_PIN = LED_OFF;
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
    Transition(state, STATE_FAULT);
    state->display = 0x3F; // Unprocessed Fault
    LED_PIN = LED_OFF;
    setPortA(state->display);
}

FaultName last_fault_sent;

void printFaultState(FaultName fault_name)
{
    // Print Door Status at the bottom left.
    if (last_fault_sent != fault_name)
    {
        SetCursorAtLine(2);
        putsLCD(getFaultStr(fault_name));
        sendToU2(getFaultStr(fault_name), STR_CODE_WIDTH);
        last_fault_sent = fault_name;
    }
}

// Perform standard transition actions
void Transition(State *state, StateName new_state)
{
        // CLEAR any bottom text
        SetCursorAtLine(2);
        putsLCD("                 ");

        // Clear U2 Commands
        resetU2();

        // Send new name to the app
        const char* state_str = getStateStr(new_state);
        sendToU2(state_str, STR_CODE_WIDTH);

        // Next state on the following tick
        state->state_name = new_state;
        state->cycle_ok = FALSE;
        timer_configured = 0;
}
