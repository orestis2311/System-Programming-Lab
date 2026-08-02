#include "bin_clock.h"

#include <avr/interrupt.h>
#include <avr/io.h>

//! Global variables
// Four LEDs
uint8_t hours;
// Six LEDs
uint8_t mins;
// Six LEDs
uint8_t secs;
//lcd only
uint16_t millis;

/*!
 * \return The milliseconds counter of the current time.
 */
uint16_t getTimeMilliseconds() {
    return millis;
}

/*!
 * \return The seconds counter of the current time.
 */
uint8_t getTimeSeconds() {
    return secs;
}

/*!
 * \return The minutes counter of the current time.
 */
uint8_t getTimeMinutes() {
    return mins;
}

/*!
 * \return The hour counter of the current time.
 */
uint8_t getTimeHours() {
    return hours;
}

/*!
 *  Initializes the binary clock (ISR and global variables)
 */
void initClock(void) {
    // Set timer mode to CTC
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);

    // Set prescaler to 1024
    TCCR0B |= (1 << CS02) | (1 << CS00);
    TCCR0B &= ~(1 << CS01);

    // Set compare register to 195 -> match every 10ms
    OCR0A = 195;

    // Time starts at 12:59:45:000
    hours = 12;
    mins = 59;
    secs = 45;
    millis = 0;

    // Enable timer and global interrupts
    TIMSK0 |= (1 << OCIE0A);
    sei();
}

/*!
 *  Updates the global variables to get a valid 12h-time
 */
void updateClock(void){
    if (millis >= 1000){
        millis -= 1000;
        secs += 1;
    }
    if (secs >= 60){
        secs -= 60;
        mins += 1;
    }
    if (mins >= 60){
        mins -= 60;
        hours += 1;
    }
    // 12:59 + 1min wraps to 1:00
    if (hours > 12){
        hours -= 12;
    }
}

/*!
 *  ISR to increase millisecond-counter of the clock
 */
ISR(TIMER0_COMPA_vect) {
    millis = millis + 10; // Called every 10ms
    updateClock(); // Handles millis >= 1000
}
