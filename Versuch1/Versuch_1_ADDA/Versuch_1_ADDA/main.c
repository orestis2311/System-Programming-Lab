#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "os_input.h"


// Require previous initio()
// Wrap in os_waitForInput
void tracking_wandler(){
	// Init Ref (R-2R)
	PORTB = 0b00000000; // Count upwards
	while (1){
		// Output to LED
		PORTA = ~PORTB;
		// Wait
		_delay_ms(50.0); // Allow comparator to comp
		// Check comp - Pin C0
		uint8_t comp = PINC & 0b00000001;
		// Count up / down
		if ((bool)comp){
			// U_ref > U_mess => Pin C0 = 1
			// Reduce U_ref
			break; // Tipping point, since we count up its finished
			//PORTB -= 1; // Unreachable
			} else {
			// U_ref < U_mess
			if (PORTB == 0b11111111) break;
			PORTB += 1;
		}
	}
	
}


void wrapped_tracking_wandler(){
	while (1){
		os_waitForInput();
		os_waitForNoInput();
		tracking_wandler();
	}
}

// Require previous initio()
// Wrap in os_waitForInput
void sa_wandler(){
	// Init Ref (R-2R)
	uint8_t curr_bit = 7;

	PORTB = 0b10000000; // Half

	while (1){
		// Output to LED (0 LED is light)
		PORTA = ~PORTB;
		// Wait
		_delay_ms(500.0); // Allow comparator to comp
		// Check comp - Pin C0
		uint8_t comp = PINC & 0b00000001;
		// Count up / down
		if ((bool)comp){
			// U_ref > U_mess => Pin C0 = 1
			// Reduce U_ref
			// Set current bit to 0, following to 1
			PORTB &= ~(1 << curr_bit);
			if (curr_bit == 0) break;
			PORTB |= (1 << (curr_bit - 1));
			
			} else {
			// U_ref < U_mess
			// Increase U_ref
			// Set following bit to 1
			if (curr_bit == 0) break;
			PORTB |= (1 << (curr_bit - 1));
		}
		curr_bit -= 1;
	}

	// Make sure to write final result to LED (0 LED is light)
	PORTA = ~PORTB;
}

void wrapped_sa_wandler(){
	while (1){
		os_waitForInput();
		os_waitForNoInput();
		sa_wandler();
	}
}

void initio(){
	// PORTD (DIP-Schalter) als Eingang
	DDRD = 0b00000000;
	// PULLUPs um DIP zu aktivieren
	PORTD = 0b11111111;
	// PORTA (LED) als Ausgang
	DDRA = 0b11111111;
	// OUTPUT to LED (None)
	PORTA = 0b11111111;
	// PORTB (R-2R) als Ausgang
	DDRB = 0b11111111;
	// OUTPUT to R-2R (None)
	PORTB = 0b00000000;
}


void manuell(void){
	while (1){
		// Read in PIND (DIP) (inverted)
		uint8_t output = ~PIND;
		// Push to LED (PORTA) and R-2R (PORTB)
		PORTA = ~output;
		PORTB = output;
		// _delay_ms(5.0);
	}
}


int main(void) {
	/* Replace with your application code */
	initio();
	
	uint8_t mode = 0; // Setzen Sie hier 0 für manuell, 1 für SA, 2 für Tracking , 3 für wrapped SA

	os_initInput();  // Initialisierung des Eingabemoduls
	
	switch (mode) {
		case 0:
		//Correct
		manuell();
		break;
		case 1:
		//Correct
		wrapped_tracking_wandler();
		break;
		case 2:
		//Correct
		wrapped_sa_wandler();
		default:
		// Fehlerbehandlung, wenn ein ungültiger Modus gewählt wird
		break;
	}
}
