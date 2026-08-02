/*! \file
 *  \brief Low level functions to draw premade things on the LED Panel
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */
#ifndef _LED_PANELDRIVER_H
#define _LED_PANELDRIVER_H
#include <avr/io.h>
#include <stdbool.h>


uint8_t framebuffer[3][16][32];


//! Initializes registers
void panel_init();

//! Starts interrupts
void panel_startTimer(void);

//! Stops interrupts
void panel_stopTimer(void);

//! Initalizes interrupt timer
void panel_initTimer(void);

void panel_latchEnable();
void panel_latchDisable();
void panel_outputEnable();
void panel_outputDisable();
void panel_setAddress(uint8_t rs);
void panel_setOutput(uint8_t layer, uint8_t row, uint8_t col);
void panel_CLK();

#endif