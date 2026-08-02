/*! \file
 *  \brief High level functions to draw premade things on the LED Panel
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */
#include "led_draw.h"
#include "led_paneldriver.h"
#include "led_patterns.h"
#include "util.h"



#define NUM_LAYERS 3
#define NUM_ROWS 16
#define NUM_COLS 32

//! \brief Distributes bits of given color's channels r, g and b on layers of framebuffer
void draw_setPixel(uint8_t x, uint8_t y, Color color) {

	if(x>=32 || x <0 || y<0 || y>=32){
		return;
	}
	
 // Determine the row (considering y > 15 for the lower half)
 uint8_t row = y % 16; // Map y to [0, 15]
 uint8_t is_upper = (y < 16) ? 1 : 0;

	 // Loop through the 3 layers
	for (uint8_t layer = 0; layer < 3; layer++) {
		 // Extract the bit for the current layer (MSB first)
		uint8_t r_bit = (color.r >> (7 - layer)) & 1;
		uint8_t g_bit = (color.g >> (7 - layer)) & 1;
		uint8_t b_bit = (color.b >> (7 - layer)) & 1;

		 // Mask for upper/lower row in the byte
		 uint8_t r_mask = is_upper ? 0x01 : 0x08; // R1 / R2
		uint8_t g_mask = is_upper ? 0x02 : 0x10; // G1 / G2
		uint8_t b_mask = is_upper ? 0x04 : 0x20; // B1 / B2

		// Clear the bits for the pixel in the framebuffer
		framebuffer[layer][row][x] &= ~(r_mask | g_mask | b_mask);

		// Set the bits for the pixel
		framebuffer[layer][row][x] |= (r_bit * r_mask) | (g_bit * g_mask) | (b_bit * b_mask);
	}
 }

//! \brief Reconstructs RGB-Color from layers of framebuffer
Color draw_getPixel(uint8_t x, uint8_t y) {
	
	if(x>=32 || x <0 || y<0 || y>=32){
		return (Color){ .r = 0, .g = 0, .b = 0 };
	}
    
	 // Determine the row (considering y > 15 for the lower half)
	 uint8_t row = y % 16; // Map y to [0, 15]
	 uint8_t is_upper = (y < 16) ? 1 : 0;

	 // Initialize the color components
	 uint8_t r = 0, g = 0, b = 0;

	 // Iterate through each layer
	 for (uint8_t layer = 0; layer < 3; ++layer) {
		 // Extract the bits for the current layer
		 uint8_t value = framebuffer[layer][row][x];

		 uint8_t r_bit = (value >> (is_upper ? 0 : 3)) & 0x01; // Extract R bit
		 uint8_t g_bit = (value >> (is_upper ? 1 : 4)) & 0x01; // Extract G bit
		 uint8_t b_bit = (value >> (is_upper ? 2 : 5)) & 0x01; // Extract B bit

		 // Combine the bits into the color intensities
		 r |= (r_bit << (7 - layer)); // Add to the correct position
		 g |= (g_bit << (7 - layer)); // Add to the correct position
		 b |= (b_bit << (7 - layer)); // Add to the correct position
	 }

	 // Return the reconstructed color
	 return (Color){ .r = r, .g = g, .b = b };
}

//! \brief Fills whole panel with given color
void draw_fillPanel(Color color) {
	// Iterate through every layer
	for (uint8_t layer = 0; layer < 3; ++layer) {
		// Extract the bit for the current layer
		uint8_t r_bit = (color.r >> (7 - layer)) & 0b00000001;
		uint8_t g_bit = (color.g >> (7 - layer)) & 0b00000001;
		uint8_t b_bit = (color.b >> (7 - layer)) & 0b00000001;

		// Create a byte pattern for the entire row in the current layer
		uint8_t row_pattern = (r_bit << 0) | (g_bit << 1) | (b_bit << 2) |
		(r_bit << 3) | (g_bit << 4) | (b_bit << 5);

		// Iterate through all rows and columns
		for (uint8_t row = 0; row < 16; ++row) {
			for (uint8_t col = 0; col < 32; ++col) {
				framebuffer[layer][row][col] = row_pattern;
			}
		}
	}
}


//! \brief Sets every pixel's color to black
void draw_clearDisplay() {
	// Iterate through all layers
	for (uint8_t layer = 0; layer < 3; ++layer) {
		// Iterate through all rows
		for (uint8_t row = 0; row < 16; ++row) {
			// Iterate through all columns
			for (uint8_t col = 0; col < 32; ++col) {
				// Clear the framebuffer for the current pixel
				framebuffer[layer][row][col] = 0b00000000;
			}
		}
	}
}


//! \brief Draws Rectangle
void draw_filledRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Color color) {
	// Ensure x1, x2, y1, y2 are within bounds and properly ordered
	if (x1 >= NUM_COLS || x2 >= NUM_COLS || y1 >= NUM_ROWS * 2 || y2 >= NUM_ROWS * 2) {
		return; // Out of bounds
	}

	if (x1 > x2) {
		uint8_t temp = x1; x1 = x2; x2 = temp; // Swap to ensure x1 <= x2
	}
	if (y1 > y2) {
		uint8_t temp = y1; y1 = y2; y2 = temp; // Swap to ensure y1 <= y2
	}

	// Iterate through all layers
	for (uint8_t layer = 0; layer < NUM_LAYERS; ++layer) {
		// Extract the bit for the current layer
		uint8_t r_bit = (color.r >> (7 - layer)) & 0b00000001;
		uint8_t g_bit = (color.g >> (7 - layer)) & 0b00000001;
		uint8_t b_bit = (color.b >> (7 - layer)) & 0b00000001;

		// Iterate over the rectangle area
		for (uint8_t y = y1; y <= y2; ++y) {
			uint8_t row = y % NUM_ROWS; // Map y to [0, 15]
			uint8_t is_upper = (y < NUM_ROWS) ? 1 : 0;

			for (uint8_t x = x1; x <= x2; ++x) {
				// Current framebuffer byte
				uint8_t *pixel = &framebuffer[layer][row][x];

				// Masks for the upper or lower row
				uint8_t r_mask = is_upper ? 0b00000001 : 0b00001000;
				uint8_t g_mask = is_upper ? 0b00000010 : 0b00010000;
				uint8_t b_mask = is_upper ? 0b00000100 : 0b00100000;

				// Clear the current bits
				*pixel &= ~(r_mask | g_mask | b_mask);

				// Set the bits for the color
				*pixel |= (r_bit * r_mask) | (g_bit * g_mask) | (b_bit * b_mask);
			}
		}
	}
}


/*! \brief Draws pattern
 * \param x			Column of left upper corner
 * \param y			Row of left upper corner
 * \param height	Height to be used for the pattern
 * \param width		Width to be used for the pattern
 * \param pattern	The given pattern. Has a maximum span of 8x8 pixel
 * \param color		RGB color used to draw the pattern with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 */
void draw_pattern(uint8_t x, uint8_t y, uint8_t height, uint8_t width, uint64_t pattern, Color color, bool overwrite) {
    uint64_t temprow = 0;
    for (uint8_t i = 0; i < height; i++) {
        // Select row
        temprow = pattern >> i * 8;

        for (uint8_t j = 0; j < width; j++) {
            if ((x + j < 32) && (y + i < 32)) {
                if (temprow & (1 << (7 - j))) {
                    draw_setPixel(x + j, y + i, color);
                } else {
                    if (overwrite) {
                        draw_setPixel(x + j, y + i, (Color){.r = 0, .g = 0, .b = 0});
                    }
                }
            }
        }
    }
}

/*! \brief Draws a character on the panel.
 * \param letter    A character of [0-9A-Za-z]. Note: small letters cannot be drawn, the corresponding capital letter will be drawn instead.
 * \param x         Column of left upper corner
 * \param y         Row of left upper corner
 * \param color     RGB color to draw letter with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 */
void draw_letter(char letter, uint8_t x, uint8_t y, Color color, bool overwrite, bool large) {
    const uint64_t *pattern_table;
    uint8_t idx;

    // the type of character determines how we get our lookup table and index
    if (letter >= '0' && letter <= '9') {
        pattern_table = large ? led_large_numbers : led_small_numbers;
        idx = letter - '0';
    } else if (letter >= 'A' && letter <= 'Z') {
        pattern_table = large ? led_large_letters : led_small_letters;
        idx = letter - 'A';
    }

    else if (letter >= 'a' && letter <= 'z') {
        pattern_table = large ? led_large_letters : led_small_letters;
        idx = letter - 'a';
    } else {
        return;
    }

    // in all cases, we have to fetch our pattern from a progmem array of patterns
    // prepare a SRAM uint64_t value to store the pattern
    uint64_t pattern;

    // Since there is no load function for uint64_ts, we will just use the Flash-to-SRAM version of memcpy
    memcpy_P(&pattern, pattern_table + idx, sizeof(uint64_t));
    draw_pattern(x, y, large ? LED_CHAR_HEIGHT_LARGE : LED_CHAR_HEIGHT_SMALL, large ? LED_CHAR_WIDTH_LARGE : LED_CHAR_WIDTH_SMALL, pattern, color, overwrite);
}


/*! \brief Draws Decimal (0..9) on panel
 * \param dec       Decimal number (from 0 to 9)
 * \param x         Row of left upper corner
 * \param y         Column of left upper corner
 * \param color     Color to draw number with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 * \param large     Draws large numbers when set to true, otherwise small (small: 5x3 px, large: 7x5 px)
 */
void draw_decimal(uint8_t dec, uint8_t x, uint8_t y, Color color, bool overwrite, bool large) {
    draw_letter(dec + '0', x, y, color, overwrite, large);
}

/*! \brief Draw an integer on the panel
 * \param number The number to draw
 * \param right_align if true, the least-significant digit of the number will be drawn at x,y; otherwise, the number will start at this position
 * \param x		   Column of the top-left corner of either the first or the last digit, depending on right_align
 * \param y		   Row of the top-left corner of either the first or the large digit, depending on right_align
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 * \param large     Draws large numbers when set to true, otherwise small (small: 5x3 px, large: 7x5 px)
 */
void draw_number(uint32_t number, bool right_align, uint8_t x, uint8_t y, Color color, bool overwrite, bool large) {
    char number_chars[10];
    uint8_t len = 0;

    uint32_t temp = number;

    while (temp >= 10) {
        number_chars[len++] = '0' + (temp % 10);
        temp /= 10;
    }
    number_chars[len++] = '0' + temp;

    const uint8_t diff = (large ? LED_CHAR_WIDTH_LARGE : LED_CHAR_WIDTH_SMALL) + 1;

    // this could potentially be <0 but unsigned integer underflow is defined
    // so that should not be a problem
    // we will simply land somewhere below 256 which will be safely cut off
    // by drawPattern, so the number is just truncated at the left
    if (right_align) x -= diff * (len - 1);
    for (uint8_t i = 0; i < len; i++) {
        draw_letter(number_chars[len - 1 - i], x + i * diff, y, color, overwrite, large);
    }
}