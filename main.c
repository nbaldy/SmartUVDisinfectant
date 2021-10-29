/*
 * 10/20/21
 */

/**
  Section: Included Files
*/
//#include "config.h" // Set CONFIG words. Be sure config.h in folder.
#include "mcc_generated_files/system.h"
#include "peripherals.h"
#include "SmartUVStateMachine.h"

/*
    Main application
 */
int main(void) {
    // initialize the device
    SYSTEM_Initialize();
    initPortA();
    initButtons(0x000F); // All buttons as inputs: 0xF
    msDelay(1000); // Give time to start up
    InitPMP();
    InitLCD();
    
    State current_state = CreateNewStateMachine();

    //endless loop
    while (1) {
        msDelay(250); // delay approximately 0.25 second
        initButtons(0x000F); // All buttons as inputs: 0xF
        
    
        processCurrentState(&current_state); // Tick State Machine
    }
}
/**
 End of File
*/