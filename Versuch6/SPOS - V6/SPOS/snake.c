#include "snake.h"
#include "joystick.h"
#include <stdlib.h>
#include <string.h>


// 32 rows, 32 columns
// Snake starts with 2 parts, shead + one snakebit
#define decrement(x) ((x) == 0 ? (x = (256 * 4) - 1) : (--(x)))

Direction inverse_direction(Direction dir){
	return (Direction)((~((uint8_t)dir))&0x03);
}

Direction get_snake_buf(uint16_t index, Snake* snek){
	uint8_t byte = index / 4;
	uint8_t offset = index % 4;

	uint8_t base = snek->buffer[byte];
	base = base >> (offset * 2); // Get the two bits in question
	base &= 0b00000011;

	return (Direction)base; // Succeeds as it is 0-3
}

void set_snake_buf(uint8_t dir, uint16_t index, Snake* snek){
	dir &= 0b00000011;
	uint8_t byte = index / 4;
	uint8_t offset = index % 4;

	uint8_t base = (*snek).buffer[byte];
	base &= ~(0b11 << (offset * 2)); // Zero the two bits in question
	base |= (dir << (offset * 2)); // Put the ones

	(*snek).buffer[byte] = base;
}

// head_idx contains the frontmost snakebit (it is meant to be shead)
void add_head(uint8_t dir, Snake* snek){
	
	(*snek).head_idx = ((*snek).head_idx + 1) % (256*4);
	set_snake_buf(dir, (*snek).head_idx, snek);
}

void cut_tail(Snake* snek){
	snek->tail_idx = (snek->tail_idx + 1) % (256*4);
}

// Standard x, y: ^|->
void add_direction(Direction dir, Position* pos){
	if (dir == JS_UP){
		pos->row += 1;
		} else if (dir == JS_DOWN){
		pos->row -= 1;
		} else if (dir == JS_LEFT){
		pos->col -= 1;
		} else if (dir == JS_RIGHT){
		pos->col += 1;
	}
}

// Checks if pos lies inside the body, does not check the snake_head itself (so it can be used for self crash)
bool inside_snake_body(Position *pos, Snake *snek){
	Position walker = {.row = snek->snake_head.row, .col = snek->snake_head.col};
	uint16_t walk_idx = snek->head_idx;
	while (walk_idx != snek->tail_idx){
		Direction walk_back = inverse_direction(get_snake_buf(walk_idx, &(*snek))); // Snake buf contains the (inverse)direction from where it came from
		add_direction(walk_back, &walker);
		if (walker.row == pos->row && walker.col == pos->col){
			return true;
		}
		decrement(walk_idx);
	}
	return false;
}

void show_snake_screen(Snake *snek, Position *food){
	// os_enterCritialSection to reduced flicker?
	draw_clearDisplay();

	Position walker = {.row = snek->snake_head.row, .col = snek->snake_head.col};
	uint16_t walk_idx = snek->head_idx;
	while (walk_idx != snek->tail_idx){
		Direction walk_back = inverse_direction(get_snake_buf(walk_idx, &(*snek))); // Snake buf contains the (inverse)direction from where it came from
		add_direction(walk_back, &walker);
		
		// col, row from top
		draw_setPixel(walker.col, 31-walker.row, COLOR_GREEN);

		decrement(walk_idx);
	}

	draw_setPixel(snek->snake_head.col, 31-snek->snake_head.row, COLOR_DARKGREEN);
	draw_setPixel(food->col, 31-food->row, COLOR_RED);
}

void show_paused_screen(uint32_t high_score, uint32_t current_score) {
	// Clear the entire display
	draw_fillPanel(COLOR_BLACK);

	// Display the high score and current score at the top
	draw_letter('H', 0, 0, COLOR_WHITE, true, false); // "H" for High Score
	draw_letter('S', 4, 0, COLOR_WHITE, true, false); // "S" for Score
	draw_letter(':', 8, 0, COLOR_WHITE, true, false);
	draw_number(high_score, false, 12, 0, COLOR_WHITE, true, false); // High Score Value

	draw_letter('C', 0, 6, COLOR_WHITE, true, false); // "C" for Current Score
	draw_letter('S', 4, 6, COLOR_WHITE, true, false);
	draw_letter(':', 8, 6, COLOR_WHITE, true, false);
	draw_number(current_score, false, 12, 6, COLOR_WHITE, true, false); // Current Score Value

	// Display "PAUSED" in the middle of the screen
	draw_letter('P', 4, 14, COLOR_WHITE, true, false);
	draw_letter('A', 8, 14, COLOR_WHITE, true, false);
	draw_letter('U', 12, 14, COLOR_WHITE, true, false);
	draw_letter('S', 16, 14, COLOR_WHITE, true, false);
	draw_letter('E', 20, 14, COLOR_WHITE, true, false);
	draw_letter('D', 24, 14, COLOR_WHITE, true, false);

	// Add a simple instruction to resume at the bottom
	/*draw_letter('R', 6, 28, COLOR_WHITE, true, false); // "R" for Resume
	draw_letter('E', 10, 28, COLOR_WHITE, true, false);
	draw_letter('S', 14, 28, COLOR_WHITE, true, false);
	draw_letter('U', 18, 28, COLOR_WHITE, true, false);
	draw_letter('M', 22, 28, COLOR_WHITE, true, false);
	draw_letter('E', 26, 28, COLOR_WHITE, true, false);*/
}

void show_dead_screen(uint32_t high_score, uint32_t current_score) {
	// Clear the entire display
	draw_fillPanel(COLOR_BLACK);

	// Display the high score and current score at the top
	draw_letter('H', 0, 0, COLOR_WHITE, true, false); // "H" for High Score
	draw_letter('S', 4, 0, COLOR_WHITE, true, false); // "S" for Score
	draw_letter(':', 8, 0, COLOR_WHITE, true, false);
	draw_number(high_score, false, 12, 0, COLOR_WHITE, true, false); // High Score Value

	draw_letter('C', 0, 6, COLOR_WHITE, true, false); // "C" for Current Score
	draw_letter('S', 4, 6, COLOR_WHITE, true, false);
	draw_letter(':', 8, 6, COLOR_WHITE, true, false);
	draw_number(current_score, false, 12, 6, COLOR_WHITE, true, false); // Current Score Value

	// Display "PAUSED" in the middle of the screen
	draw_letter('D', 8, 14, COLOR_RED, true, false);
	draw_letter('E', 12, 14, COLOR_RED, true, false);
	draw_letter('A', 16, 14, COLOR_RED, true, false);
	draw_letter('D', 20, 14, COLOR_RED, true, false);
	


}




uint8_t snake_Game(uint8_t hs){
	
	
	Snake snek = {
		.head_idx = 0,
		.tail_idx = 0,
		.buffer = {0},
		.snake_head = {16, 16}
	};
	
	set_snake_buf((uint8_t)JS_UP, 0, &snek);

	add_head((uint8_t)JS_UP, &snek);
	
	/*
	snek.buffer[0] = 0xFF;
	snek.buffer[1] = 0b00001010;
	snek.head_idx = 4;
	snek.tail_idx = 0;
	increment_head(&snek);
	increment_head(&snek);
	set_snake_buf(2, 6, &snek);
	increment_head(&snek);
	set_snake_buf(2, 7, &snek);
	add_head(0, &snek);
	
	if (snek.head_idx != 8){
		draw_fillPanel(COLOR_RED);
		while(1);
	}*/
	
	//--
	//add_head(0b00000011, &snek);

	// New food
	Position food;
	food.row = rand() % 32;
	food.col = rand() % 32;
	//while (inside_snake_body(&food, &snek) || (snek.snake_head.row == food.row && snek.snake_head.col == food.col)){
		//food.row = rand() % 32;
		//food.col = rand() % 32;
	//}
	uint8_t high_score = hs;
	uint8_t current_score = 0;
	Direction current_dir = JS_UP;
	bool crash = false;
	
	show_snake_screen(&snek, &food);

	while (1){
		show_snake_screen(&snek, &food);
		delayMs(500); // Tick delay

		
		if (js_getButton()){
			
			show_paused_screen(high_score,current_score);
			while (js_getButton()); // Stuck here during first press

			while (!js_getButton()); // Stuck here after first button release; GAME PAUSED NO BUTTON PRESSED
			
			// Button got pressed down a second time, since no longer stuck
			show_snake_screen(&snek, &food);
			while (js_getButton()); // Stuck here during second press
			
			
			// 100ms after second button release
			delayMs(100);
		}
		

		// Display
		// Get direction
		// Move head
		// Check head, food? skip cut
		// Check head, wall? crash
		// Check head, self? crash
		// Cut tail

		Direction new_direction = js_getDirection();
		if ((new_direction != JS_NEUTRAL) && (new_direction != inverse_direction(current_dir))){
			current_dir = new_direction;
		}

		// Move the head of the snake
		add_direction(current_dir, &(snek.snake_head));
		add_head(current_dir, &snek);

		// Found food
		if (snek.snake_head.row == food.row && snek.snake_head.col == food.col){
			current_score += 1;
			if (current_score > high_score) high_score = current_score;
			// 256 - 2 starting snakebits -- full?
			if (current_score == 254){
				crash = true;
				} else {
				// New food
				food.row = rand() % 32;
				food.col = rand() % 32;
				while (inside_snake_body(&food, &snek) || (snek.snake_head.row == food.row && snek.snake_head.col == food.col)){
					food.row = rand() % 32;
					food.col = rand() % 32;
				}
			}
			
			// Dont cut
			continue;

			// Overflow means only one-sided check
			} else if (snek.snake_head.row >= 32 || snek.snake_head.col >= 32){
			// DO WALL CRASH
			crash = true;
			} else {
			// SELF CRASH CHECK
			crash = inside_snake_body(&(snek.snake_head), &snek);
		}


		if (crash){
			show_dead_screen(high_score,current_score);
			while (!js_getButton());
			break;
		}
		cut_tail(&snek);
	}
	return high_score;
}

// REGISTER AUTOSTART THIS
void looping_snake_game(){
	panel_init();
	js_init();

	panel_initTimer();
	panel_startTimer();
	
	uint8_t hs = 0;
	while (1)
	{
		hs = snake_Game(hs);
	}
	
}