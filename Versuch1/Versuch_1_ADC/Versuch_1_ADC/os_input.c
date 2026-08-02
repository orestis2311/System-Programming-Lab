#include "os_input.h"
#include <avr/io.h>
#include <stdint.h>

/*!
 *  A simple "Getter" function for the Buttons on the evaluation board.
 *  \returns The state of the button(s) in the lower bits of the return value.
 */
uint8_t os_getInput(void) {
    uint8_t buttonState = 0;

    // Read specific button states and map them to the lower 4 bits of buttonState
    if (!(PINC & (1 << PC0))) buttonState |= (1 << 0);  // Enter button (C0)
    if (!(PINC & (1 << PC1))) buttonState |= (1 << 1);  // Down button (C1)
    if (!(PINC & (1 << PC6))) buttonState |= (1 << 2);  // Up button (C6)
    if (!(PINC & (1 << PC7))) buttonState |= (1 << 3);  // ESC button (C7)

    return buttonState;
}

/*!
 *  Initializes DDR and PORT for input.
 */
void os_initInput() {
    // Configure Port C pins as input for the buttons (C0, C1, C6, C7)
    DDRC &= ~(1 << PC0);   // Set PC0 as input
    DDRC &= ~(1 << PC1);   // Set PC1 as input
    DDRC &= ~(1 << PC6);   // Set PC6 as input
    DDRC &= ~(1 << PC7);   // Set PC7 as input

    // Enable pull-up resistors on Port C for the buttons
    PORTC |= (1 << PC0);   // Enable pull-up on PC0
    PORTC |= (1 << PC1);   // Enable pull-up on PC1
    PORTC |= (1 << PC6);   // Enable pull-up on PC6
    PORTC |= (1 << PC7);   // Enable pull-up on PC7
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void os_waitForNoInput() {
    // Loop until all buttons are released (all corresponding bits read as 1)
    while ((PINC & ((1 << PC0) | (1 << PC1) | (1 << PC6) | (1 << PC7))) !=
           ((1 << PC0) | (1 << PC1) | (1 << PC6) | (1 << PC7))) {
        // Do nothing, just wait
    }
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void os_waitForInput() {
    // Loop until any button is pressed (any corresponding bit reads as 0)
    while ((PINC & ((1 << PC0) | (1 << PC1) | (1 << PC6) | (1 << PC7))) ==
           ((1 << PC0) | (1 << PC1) | (1 << PC6) | (1 << PC7))) {
        // Do nothing, just wait
    }
}
