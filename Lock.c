// Implementation file for lock

#include "Lock.h"
#include "peripherals.h"

#include <xc.h> // include processor files - each processor file is guarded.  

void InitLock()
{
    LOCK_TRIS = 0; // Output
}

void Lock()
{
    LOCK_PIN = 1;
}

void Unlock()
{
    LOCK_PIN = 0;
}