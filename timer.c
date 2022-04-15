// Implementation file for timer

#include "timer.h"
#include "peripherals.h"

#include <xc.h> // include processor files - each processor file is guarded.  

int _num_seconds_needed = 0;
int _num_seconds_detected = 0;
LongTimerStatus _status = STATUS_UNKNOWN;

void ConfigureLongTimer(int s)
{
    // PRESCALER MUST BE CONFIGURED TO 256 
    T1CON = 0x8030;// T1 on, prescaler 256
    _status = STATUS_CONFIGURED;
    _num_seconds_needed = s;
    _num_seconds_detected = 0;
}

void StartLongTimer(void)
{
    TMR1 = 0;
    _status = STATUS_RUNNING;
    _num_seconds_detected = 0;
}

enum LongTimerStatus CheckTimerStatus(void)
{
    if (T1CON != 0x8030)
    {
        // Changed prescaler detected
        _status = STATUS_ERR;
        return _status;        
    }

    if (TMR1 > TMR1_1SEC_MAX_PRESCALE)
    {
        // Start next cycle
        _num_seconds_detected++;
        TMR1 -= TMR1_1SEC_MAX_PRESCALE;
        _status = STATUS_RUNNING; // Do not detect a rollover
    }
    else if (TMR1 > TMR1_HALFWAY_1S)
    {
        _status = STATUS_PAST_HALFWAY;
    }
    else if (_status == STATUS_PAST_HALFWAY)
    {
        // Rollover detected
        TMR1 += (TMR_MAX - TMR1_1SEC_MAX_PRESCALE);
        _num_seconds_detected++;
        _status = STATUS_RUNNING; // Rollover processed
    }

    if (_num_seconds_detected >= _num_seconds_needed)
    {
        _status = STATUS_DONE;
    }

    return _status;
}

int GetSecondsElapsed(void)
{
    return _num_seconds_detected;
}

int GetSecondsRemaining(void)
{
    return _num_seconds_needed - _num_seconds_detected;
}