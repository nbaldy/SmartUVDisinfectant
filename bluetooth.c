/*
 * File:   bluetooth.c
 * Author: ljjr8
 *
 * Created on February 21, 2022, 5:21 PM
 */

#include <xc.h>
#include "bluetooth.h"
#include "peripherals.h"

#define RTS _RF13 // Output, For potential hardware handshaking.
#define CTS _RF12 // Input, For potential hardware handshaking.

#define ASTERISK 0x2A
#define CARRIAGE 0x0D
#define NEWLINE 0x0A
#define NULL 0x00

#define CHAR_A 0x41
#define CHAR_Z 0x5a
#define CHAR_a 0x61
#define CHAR_z 0x7a

#define FLUSH_BUFFER() U2STAbits.OERR = 0

// UART 
void InitU2(void)
{
    U2BRG = 34; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy= 16MHz.
    U2MODE =0x8008; // See data sheet, pg148. Enable UART2, BRGH = 1,
    // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
    // Following lines pertain Hardware handshaking
    TRISFbits.TRISF13 = 1; // enable RTS , output

    RTS = 1; // default status , not ready to send
    FLUSH_BUFFER();
}

char putU2(char c)
{
    while (CTS); //wait for !CTS (active low)
    while (U2STAbits.UTXBF ); // Wait if transmit buffer full.
    U2TXREG = c; // Write value to transmit FIFO
    return c;
}

char getU2( void )
{
    RTS = 0; // telling the other side !RTS
    unsigned int max_tries = 100;
    while (!U2STAbits. URXDA && max_tries-- > 0);
    if (!U2STAbits. URXDA)
    {
        RTS = 1;
        return NEWLINE;
    }
    RTS = 1; // telling the other side RTS
    return U2RXREG ; // from receiving buffer
} //getU2

// Bluetooth Communication
int checkCommand(char c)
{
    if (c == CHAR_a + 18)           // checks if command was START
    {
        return START_CMD;
    }
    else if (c == CHAR_a + 4)      // checks if command was END
    {
        return END_CMD;
    }
    else if (c == CHAR_a + 17)      // checks if command was RELEASE
    {
        return RELEASE_CMD;
    }
    else if (c == CHAR_a + c)      // checks if command was CLEAR (fault))
    {
        return CLR_CMD;
    }

    else
    {
        return c;
    }
}

int getCommand ()
{
    int command = NO_CMD;

    int isCommand = 0;
    char str[3];
    int size = 0;

    unsigned int max_letters = 5; // Includes asterisks

    while(command == NO_CMD && max_letters-- > 0)
    {
        char temp;
        temp = getU2();
        msDelay(50);

        // Checks if char is asterisk
        if (temp == ASTERISK)
        {
            isCommand = !isCommand;     //sets command flag

            if (isCommand)
            {
                // Start of a command
                continue;
            }
            else
            {
                // End of a command
                str[size] = NULL;
                putU2(CARRIAGE);
                msDelay(50);
                putU2(NEWLINE);
                msDelay(50);
                FLUSH_BUFFER();
                command = checkCommand(str[0]);
                return command;
            }
        }
        else if (isCommand && temp != NEWLINE)  // Middle of a command
        {
            str[size] = temp;
            putU2(temp);
            size++;
        }
    }

    FLUSH_BUFFER();
    return command;
}

void resetU2()
{
    FLUSH_BUFFER();
}

int isLetter(char c)
{
    return ((c >= CHAR_A && c <= CHAR_Z) || (c >= CHAR_a && c <= CHAR_z));
}

void sendToU2(const char str[], unsigned int size)
{
    unsigned int i;
    for (i=0; i < size; i++)
    {
        putU2(str[i]);
        msDelay(10);
    }
    putU2(CARRIAGE);
    msDelay(10);
    putU2(NEWLINE);
    msDelay(10);
}