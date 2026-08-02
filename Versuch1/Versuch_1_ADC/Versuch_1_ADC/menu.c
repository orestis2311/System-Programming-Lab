#include "menu.h"

#include "adc.h"
#include "bin_clock.h"
#include "lcd.h"
#include "led.h"
#include "os_input.h"
#include <util/delay.h>

#include <avr/io.h>
#include <stdint.h>

/*!
 *  Hello world program.
 *  Shows the string 'Hello World!' on the display.
 */
// Require previous lcd_init()
void helloWorld(void) {
    // Repeat until ESC gets pressed

    
    while (1){
        lcd_writeString("Hello World");
        _delay_ms(500.0);
        lcd_clear();
        _delay_ms(500.0);
        // C7 - ESC pushed?
        if ((os_getInput()&0b00001000) != 0){
            break;
        }
    }
}

/*!
 *  Shows the clock on the display and a binary clock on the led bar.
 */
void displayClock(void) {
	while(1){
	updateClock();
    uint16_t clockVal = 0;
    // Hours (4bits)
    clockVal |= (uint16_t)(getTimeHours() & 0b00001111);

    // Mins (6bits)
    clockVal = clockVal << 6;
    clockVal |= (uint16_t)(getTimeMinutes() & 0b00111111);

    // Secs (6bits)
    clockVal = clockVal << 6;
    clockVal |= (uint16_t)(getTimeSeconds() & 0b00111111);

    // Output to LEDs
    setLedBar(clockVal);

    // Output to lcd
    lcd_clear();
    lcd_line1();

    // Pad with 0s
    if (getTimeHours() < 10){
        lcd_writeChar('0');
    }
    lcd_writeDec((uint16_t)getTimeHours());
    lcd_writeChar(':');
    
    if (getTimeMinutes() < 10){
        lcd_writeChar('0');
    }
    lcd_writeDec((uint16_t)getTimeMinutes());
    lcd_writeChar(':');

    if (getTimeSeconds() < 10){
        lcd_writeChar('0');  
    }
    lcd_writeDec((uint16_t)getTimeSeconds());
    lcd_writeChar(':');
    
    if (getTimeMilliseconds() < 100){
        lcd_writeChar('0');
        if (getTimeMilliseconds() < 10){
            lcd_writeChar('0');
        }
    }
    lcd_writeDec((uint16_t)getTimeMilliseconds());
	}
}

/*!
 *  Shows the stored voltage values in the second line of the display.
 */
void displayVoltageBuffer(uint8_t displayIndex) {
    lcd_line2();

    if (displayIndex < 10){
        lcd_writeChar('0');
    }
    lcd_writeDec(displayIndex);
    lcd_writeChar('/');
    lcd_writeDec(100);
    lcd_writeChar(' ');
    uint16_t stored_voltage = getStoredVoltage(displayIndex);
    lcd_writeVoltage(stored_voltage , 1023 , 5);
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {

	uint8_t count = 0;
	while(1){
		
		uint16_t voltage = getAdcValue();
		
		lcd_line1();
		lcd_writeProgString(PSTR("Voltage: "));
		lcd_writeVoltage(voltage , 1023 , 5);
		if ((os_getInput()&0b00001000) != 0){
			break;
		}
		
		 //1023 every 68 => 1 led
		uint16_t ledValue = 0;
		  while(1){
			  if(voltage <= 68) break;
			  
			  ledValue = ledValue << 1;
			  ledValue |= 2;
			  voltage -= 68;
		  }
		setLedBar(ledValue);

        // Enter to save
        if ((os_getInput()&0b00000001) != 0){
            // Save voltage
            storeVoltage();
        // Down to dec count
        } else if ((os_getInput()&0b00000010) != 0){
            count -= 1;
        // Up to inc count
        } else if ((os_getInput()&0b00000100) != 0){
            count += 1;
        }
        displayVoltageBuffer(count);
        _delay_ms(100.0);
	}
}

/*! \brief Starts the passed program
 *
 * \param programIndex Index of the program to start.
 */
void start(uint8_t programIndex) {
    // Initialize and start the passed 'program'
    switch (programIndex) {
        case 0:
			//Correct
            lcd_clear();
            helloWorld();
            break;
        case 1:
			//Correct
            activateLedMask = 0xFFFF; // Use all LEDs
            initLedBar();
            initClock();
            displayClock();
            break;
        case 2:
            activateLedMask = 0xFFFE; // Don't use LED 0
            initLedBar();
            initAdc();
            displayAdc();
            break;
        default:
            break;
    }

    // Do not resume to the menu until all buttons are released
    os_waitForNoInput();
}

/*!
 *  Shows a user menu on the display which allows to start subprograms.
 */
void showMenu(void) {
    uint8_t pageIndex = 0;

    while (1) {
        lcd_clear();
        lcd_writeProgString(PSTR("Select:"));
        lcd_line2();

        switch (pageIndex) {
            case 0:
                lcd_writeProgString(PSTR("1: Hello world"));
                break;
            case 1:
                lcd_writeProgString(PSTR("2: Binary clock"));
                break;
            case 2:
                lcd_writeProgString(PSTR("3: Internal ADC"));
                break;
            default:
                lcd_writeProgString(PSTR("----------------"));
                break;
        }

        os_waitForInput();
        if (os_getInput() == 0b00000001) { // Enter
            os_waitForNoInput();
            start(pageIndex);
        } else if (os_getInput() == 0b00000100) { // Up
            os_waitForNoInput();
            pageIndex = (pageIndex + 1) % 3;
        } else if (os_getInput() == 0b00000010) { // Down
            os_waitForNoInput();
            if (pageIndex == 0) {
                pageIndex = 2;
            } else {
                pageIndex--;
            }
        }
    }
}
