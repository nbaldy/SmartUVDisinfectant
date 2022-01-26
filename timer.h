/* 
 * File:   long_timer.cpp
 * Author: Nicole Baldy
 *
 * Keep track of long periods of time (up to 5 minutes) without blocking. 
 * Eventually, all timers should use this value. 
 * TODO(NEB): Change to interrupts
 * Created on January 24, 2022, 2:40 PM
 */

// TODO(NEB): cannot keep track of us and below with us prescale, 
// so we need to be careful about overwriting the configuration
#ifndef TIMER_H
#define	TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MAX_PRESCALE 256
#define TMR1_1SEC_MAX_PRESCALE 62500 // 1 / (256/16,000,000)
#define TMR1_HALFWAY_1S TMR1_1SEC_MAX_PRESCALE / 2 // Helps detect rollovers

# define TMR_MAX 65535 // 2^16 - 1

typedef enum LongTimerStatus
{
    STATUS_UNKNOWN = 0,
    STATUS_CONFIGURED, // conf but not running
    STATUS_RUNNING,
    STATUS_PAST_HALFWAY, // Safety net to catch overflow situations 
    STATUS_DONE, 
    STATUS_ERR // TODO(NEB): Check T1Con to ensure nothing else messes with the prescaler
} LongTimerStatus;

// The long timer is nonblocking.
// It is currently safe with ms delay calls but NOT us calls. 
// This is because us delay calls actually use the timer. 
// TODO: Might be easiest just to use # of instructions for small delays, though this sacrifices accuracy
void ConfigureLongTimer(int sec);
void StartLongTimer(void);
// Timer status assumed to be checked at a rate > 1/3 sec
enum LongTimerStatus CheckTimerStatus(void);
int GetSecondsElapsed(void);

extern int _num_seconds_needed; // Set by config.
extern int _num_seconds_detected; // Updated each check.
extern LongTimerStatus _status;

#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */
