/*
 * 10/20/21
 */

/**
  Section: Included Files
*/
//#include "config.h" // Set CONFIG words. Be sure config.h in folder.
#include "mcc_generated_files/system.h"
#include "peripherals.h"

/*
                         Main application
 */
int main(void) {
    // initialize the device
    SYSTEM_Initialize();
    unsigned char display = 0;
    initPortA();
    initButtons(0x000F); // All buttons as inputs: 0xF
    //endless loop
    while (1) {
        msDelay(1000); // delay approximately 1 second
        // display += 0x1;
        initButtons(0x000F); // All buttons as inputs: 0xF
        if (getButton(0x0001))
            display = 0x01; //if S4 pressed, show 1
        else
            display &= 0xFE; // 1st LED off if not pressed
        
        if (getButton(0x0002))
            display = 0x02; //if S5 pressed, show 2
        else
            display &= 0xFD; // 2nd LED off if not pressed

        if (getButton(0x0004))
            display = 0x04; //if B4 pressed, show 8
        else
            display &= 0xFB; // 3rd LED off if not pressed

        if (getButton(0x0008))
            display = 0x08; // else if RD6 pressed, show 8
        else
            display &= 0xF7; // 3rd LED off if not pressed

        setPortA(display); // sent count to display
    }
}
/**
 End of File
*/