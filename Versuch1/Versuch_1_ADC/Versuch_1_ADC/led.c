#include "led.h"

#include <avr/io.h>

uint16_t activateLedMask = 0xFFFF;

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {

    // LEDs connected on A and D
    DDRA |= 0b11111111 & (uint8_t)activateLedMask;
    DDRD |= 0b11111111 & (uint8_t)(activateLedMask >> 8);
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {
    // LEDs light up on 0. Should light up on parameter's 1
	uint16_t to_set = (~value);
    // value[15..=8] für PORTD value[7..=0] für PORTA
    uint8_t upper_split = (uint8_t)(to_set >> 8);
    uint8_t lower_split = (uint8_t)to_set;

    // Put 0s, Mask forces 1 on inactive
	PORTD &= upper_split | ~((uint8_t)(activateLedMask >> 8));
    PORTA &= lower_split | ~((uint8_t)activateLedMask);

    // Put 1s, Mask forces 0 on inactive
	PORTD |= upper_split & ((uint8_t)(activateLedMask >> 8));
    PORTA |= lower_split & ((uint8_t)activateLedMask);
}
