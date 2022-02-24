/* 
 * File:   bluetooth.h
 * Author: ljjr8
 *
 * Created on February 21, 2022, 5:20 PM
 */

#ifndef BLUETOOTH_H
#define	BLUETOOTH_H

#define START_CMD 1
#define END_CMD 2
#define RELEASE_CMD 3
#define NO_CMD 0

#ifdef	__cplusplus
extern "C" {
#endif

    // UART
    void InitU2(void);
    char putU2(char c);
    char getU2(void);
    
    // Bluetooth Communication
    int checkCommand (char c);
    int getCommand(void);
    int isLetter(char c);
    
    void resetU2(void);

#ifdef	__cplusplus
}
#endif

#endif	/* BLUETOOTH_H */

