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
    int n;
    for (n = 0; n < 2000; n++)
    {
        LOCK_PIN = 0; // Lock is non-powered
    }
}

void Unlock()
{
    LOCK_PIN = 1; // Lock is powered
}