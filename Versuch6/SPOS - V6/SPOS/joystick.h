/*
 * joystick.h
 *
 * Created: 24.01.2025 18:25:23
 *  Author: uo204564
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_


#include "defines.h"
#include "util.h"

#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>


typedef enum {
	JS_LEFT = 0b00000010,
	JS_RIGHT = 0b00000001,
	JS_UP = 0b00000011,
	JS_DOWN = 0b00000000,
	JS_NEUTRAL = 0b00000100,
} Direction;


void js_init(void);


uint16_t js_getHorizontal(void);
uint16_t js_getVertical(void);

Direction js_getDirection(void);

bool js_getButton(void);



#endif /* JOYSTICK_H_ */