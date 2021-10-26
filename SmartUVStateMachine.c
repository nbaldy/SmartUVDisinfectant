// Implementation file for state machine

#include "SmartUVStateMachine.h"
#include "peripherals.h"

struct State CreateNewStateMachine()
{
    State new_sm;
    new_sm.display = 0x00;
    new_sm.state_name = STATE_INITIALIZATION;
    return new_sm;
}

void processCurrentState(State* current_state)
{
    // Rebuilt display with each tick (TODO - reconsider this.)
    current_state->display = 0x00;

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
        return;
    }
    
    // TODO(NEB): Initialize all sensors.
    state->display |= 0x01;
}

void WaitForObject(State* state)
{
    Running(state); // Parent State
    if (getButton(BUTTON_READY_FOR_NEXT))
    {
        // NOTE(NEB): For now, consider "door closed" when button pressed.
        // Event: Door Closed
        state->state_name = STATE_VERIFY_CHAMBER_READY;
        return;
    }
    
    // TODO(NEB): Wait for door closed sense
    state->display |= 0x02;
}

void VerifyChamberReady(State* state)
{
    Running(state); // Parent State
    if (getButton(BUTTON_READY_FOR_NEXT))
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
    if (getButton(BUTTON_READY_FOR_NEXT))
    {
        // NOTE(NEB): For now, consider "stopped" when button pressed.
        // Event: EITHER Stop Cmd Recieved OR Timer Expired
        state->state_name = STATE_WAIT_FOR_RELEASE;
        return;
    }
    
    // TODO(NEB): Wait for stop cmd from wireless OR timer complete
    state->display |= 0x05;
}

void WaitForRelease(State* state)
{
    Running(state); // Parent State
    if (getButton(BUTTON_READY_FOR_NEXT))
    {
        // NOTE(NEB): For now, consider "unlocked" when button pressed.
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
        state->state_name = STATE_INITIALIZATION;
        return;
    }

    state->display = 0x7F; // Active Fault
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
}

void SetFault(State *state)
{
    state->state_name = STATE_FAULT;
    state->display = 0x3F; // Unprocessed Fault
    setPortA(state->display);
}
