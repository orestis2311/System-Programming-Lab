#ifndef SNAKE_H
#define SNAKE_H
#include "led_paneldriver.h"
#include "led_patterns.h"
#include "led_draw.h"
#include "util.h"
#include "joystick.h"

typedef struct {
	uint8_t row;
	uint8_t col;
} Position;

typedef struct {
	uint16_t head_idx;
	uint16_t tail_idx;
	uint8_t buffer[256]; // 4 indices per byte
	Position snake_head;
} Snake;



Direction inverse_direction(Direction dir);
Direction get_snake_buf(uint16_t index, Snake* snek);

void set_snake_buf(uint8_t dir, uint16_t index, Snake* snek);
void add_head(uint8_t dir, Snake* snek);

void cut_tail(Snake* snek);
void add_direction(Direction dir, Position* pos);
bool inside_snake_body(Position *pos, Snake *snek);
void show_snake_screen(Snake *snek, Position *food);

void show_paused_screen();

uint8_t snake_Game(uint8_t hs);


void looping_snake_game();
#endif