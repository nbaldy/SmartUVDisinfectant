/* 
 * File:  Lock control 
 * Author: Nicole Baldy
 * Comments: Control a digital lock
 */

#ifndef Lock_H
#define	Lock_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define LOCK_PIN PORTGbits.RG15
#define LOCK_TRIS TRISGbits.TRISG15


// Get lock ready for use
void InitLock();

// Activate the lock
void Lock();

// Deactivate the lock
void Unlock();

#endif	/* Lock_H */

