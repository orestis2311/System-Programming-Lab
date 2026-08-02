#include "joystick.h"
#include <avr/io.h>

#include "defines.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>


void js_mux_select(uint8_t target){
	if (target >= (1 << 5)) return;
	ADMUX |= (0b00011111 & target); // Put target 0s
	ADMUX &= (0b11100000 | target); // Put target 1s
}


void js_init() {
	DDRA &= 0b00011111; //A5-A7 as Joystick     (input)
	PORTA |= 0b11100000;
	// Configure ADC
	ADMUX = (1 << REFS0); // Set reference voltage to VCC
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler = 64
}


uint16_t js_getHorizontal() {
	js_mux_select(5); // Select channel A5
	ADCSRA |= (1 << ADSC);         // Start conversion
	while ((ADCSRA & (1 << ADSC)) != 0);  // Wait for conversion to complete
	return ADC;                    // Return the ADC value
}

uint16_t js_getVertical() {
	js_mux_select(6); // Select channel A6
	ADCSRA |= (1 << ADSC);         // Start conversion
	while ((ADCSRA & (1 << ADSC)) != 0);  // Wait for conversion to complete
	return ADC;                    // Return the ADC v	panel_init()
}

Direction js_getDirection() {
	uint16_t horizontal = js_getHorizontal();
	uint16_t vertical = js_getVertical();

	// Define thresholds
	const uint16_t neutralMin = 450; // ~2.2V
	const uint16_t neutralMax = 600; // ~2.8V

	// Horizontal axis
	if (horizontal < neutralMin) return JS_LEFT;
	if (horizontal > neutralMax) return JS_RIGHT;

	// Vertical axis
	if (vertical < neutralMin) return JS_DOWN;
	if (vertical > neutralMax) return JS_UP;

	// Neutral position
	return JS_NEUTRAL;
}

bool js_getButton() {
	return ((PINA & (1 << PA7)) == 0); // Button is active low
}