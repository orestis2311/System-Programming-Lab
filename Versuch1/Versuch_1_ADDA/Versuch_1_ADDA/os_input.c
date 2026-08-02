#include "os_input.h"

#include <avr/io.h>
#include <stdint.h>

/*! \file
 * Everything that is necessary to get the input from the Buttons in a clean format.
 */


/*!
 *  A simple "Getter"-Function for the Buttons on the evaluation board.\n
 *
 *  \returns The state of the button in the lower bits of the return value.\n
 *  example: Button C1: -pushed:   00000001
 *                      -released: 00000000
 */
uint8_t os_getInput(void) {
    // Button not inverted?
    return PINC; 
}


/*!
 *  Initializes DDR and PORT for input
 */
void os_initInput() {
    DDRC = 0b00000000;
    PORTC = 0b00000011;
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void os_waitForNoInput() {
    while(1){
        // Check Button on C1 == 0
        if ((os_getInput()&0b00000010) == 0){
            break;
        } 
    }
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void os_waitForInput() {
    while(1){
        // Check Button on C1 != 0
        if ((os_getInput()&0b00000010) != 0){
            break;
        }
    }
}