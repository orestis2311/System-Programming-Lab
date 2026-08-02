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
    if (!(PINB & (1 << PB0))) buttonState |= (1 << 0);  // Enter button (B0)
    if (!(PINB & (1 << PB1))) buttonState |= (1 << 1);  // Down button (B1)
    if (!(PINB & (1 << PB6))) buttonState |= (1 << 2);  // Up button (B6)
    if (!(PINB & (1 << PB7))) buttonState |= (1 << 3);  // ESC button (B7)

    return buttonState;
}

/*!
 *  Initializes DDR and PORT for input.
 */
void os_initInput() {
    // Configure Port B pins as input for the buttons (B0, B1, B6, B7)
    DDRB &= ~(1 << PB0);   // Set PB0 as input
    DDRB &= ~(1 << PB1);   // Set PB1 as input
    DDRB &= ~(1 << PB6);   // Set PB6 as input
    DDRB &= ~(1 << PB7);   // Set PB7 as input

    // Enable pull-up resistors on Port B for the buttons
    PORTB |= (1 << PB0);   // Enable pull-up on PB0
    PORTB |= (1 << PB1);   // Enable pull-up on PB1
    PORTB |= (1 << PB6);   // Enable pull-up on PB6
    PORTB |= (1 << PB7);   // Enable pull-up on PB7
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void os_waitForNoInput() {
    // Loop until all buttons are released (all corresponding bits read as 1)
    while ((PINB & ((1 << PB0) | (1 << PB1) | (1 << PB6) | (1 << PB7))) !=
           ((1 << PB0) | (1 << PB1) | (1 << PB6) | (1 << PB7))) {
        // Do nothing, just wait
    }
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void os_waitForInput() {
    // Loop until any button is pressed (any corresponding bit reads as 0)
    while ((PINB & ((1 << PB0) | (1 << PB1) | (1 << PB6) | (1 << PB7))) ==
           ((1 << PB0) | (1 << PB1) | (1 << PB6) | (1 << PB7))) {
        // Do nothing, just wait
    }
}
