/*! \file
 *  \brief Functions to draw premade things on the LED Panel
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */
#include "led_paneldriver.h"

#include "defines.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

#define LAYER_CYCLE_LENGTH 7
uint8_t layer_cycle[LAYER_CYCLE_LENGTH] = {0,1,0,2,0,1,0};
uint8_t isr_layer = 0;
uint8_t isr_row = 0;


//! \brief Enable compare match interrupts for Timer 1
void panel_startTimer() {
    sbi(TIMSK1, OCIE1A);
}

//! \brief Disable compare match interrupts for Timer 1
void panel_stopTimer() {
    cbi(TIMSK1, OCIE1A);
}

//! \brief Initialization function of Timer 1
void panel_initTimer() {
    // Configuration TCCR1B register
    sbi(TCCR1B, WGM12); // Clear on timer compare match
    sbi(TCCR1B, CS12);  // Prescaler 256  1
    cbi(TCCR1B, CS11);  // Prescaler 256  0
    cbi(TCCR1B, CS10);  // Prescaler 256  0

    // Output Compare register 256*7 = 1792 tics => interrupt interval approx 0.0896 ms
    OCR1A = 0x0007;
}

void framebuffer_init() {
	for (uint8_t layer = 0; layer < 3; ++layer) {
		for (uint8_t row = 0; row < 16; ++row) {
			for (uint8_t col = 0; col < 32; ++col) {
				framebuffer[layer][row][col] = 0b00000000; // Clear all pixels
			}
		}
	}
}

// TODO Call from os_init
//! \brief Initializes used ports of panel
// Pin definitions based on the document
#define CLK_PIN PC0
#define LE_PIN PC1
#define OE_PIN PC6
#define ROW_SELECT_MASK 0x0F // Pins PA0-PA3
#define DATA_PORT PORTD    // Data pins: PD0-PD5

void panel_init() {
	// Set row select pins (PA0-PA3) as outputs
	DDRA |= ROW_SELECT_MASK;

	// Set control pins (CLK, LE, OE) as outputs
	DDRC |= (1 << CLK_PIN) | (1 << LE_PIN) | (1 << OE_PIN);

	// Set data pins (PD0-PD5) as outputs
	DDRD |= 0x3F; // Pins PD0 to PD5

	// Initialize all control pins to their default states
	PORTC &= ~((1 << CLK_PIN) | (1 << LE_PIN) | (1 << OE_PIN));

	// Initialize row select to 0
	PORTA &= ~ROW_SELECT_MASK;

	// Initialize data pins to 0
	DATA_PORT &= ~0x3F;
}

void panel_latchEnable(){
    PORTC |= 0b00000010;
}
void panel_latchDisable(){
    PORTC &= ~0b00000010;
}
void panel_outputEnable(){
    PORTC &= 0b10111111;
}
void panel_outputDisable(){
    PORTC |= ~0b10111111;
}
void panel_setAddress(uint8_t rs){
    rs &= 0x0F;
    PORTA &= (0xF0)|rs; // Put rs zeros
    PORTA |= rs;        // Put rs ones
}
void panel_setOutput(uint8_t layer, uint8_t row, uint8_t col){
    PORTD = framebuffer[layer][row][col]; // D6, D7 are free so who cares
}
void panel_CLK(){
    PORTC |= 0x01; // Tick
    PORTC &= ~0x01; // Boom?
}




//! \brief ISR to refresh LED panel, trigger 1 compare match interrupts
ISR(TIMER1_COMPA_vect) {
    // Elect layer
    // Elect d-row
    // Set target d-row (for the matrix)
    // Push all bits for the d-row into shift regs with clk ticks
    // Save into latch
    // Activate output - deactivate once before use
    panel_outputDisable(); // Give it a short time to cool down
    uint8_t layer = layer_cycle[isr_layer];
    isr_layer = (isr_layer + 1)%LAYER_CYCLE_LENGTH;
    uint8_t d_row = isr_row;
    isr_row = (isr_row + 1)%16;

    panel_setAddress(d_row);
    for (uint8_t i = 0; i<32; i++){
        panel_setOutput(layer, d_row, i);
        panel_CLK();
    }
    panel_latchEnable();
    panel_latchDisable();
    panel_outputEnable();
}