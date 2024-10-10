#include <stdio.h>
#include <stdlib.h>
#include "ws2812b.h"
#include "spi.h"
#include "tim.h"

const uint8_t gamma_table[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

uint32_t compute_color(uint8_t r, uint8_t g,uint8_t b){
	r = gamma_table[r];
	g = gamma_table[g];
	b = gamma_table[b];

	return ((uint32_t)(b | (g<<16) | (r<<8)));
}
void breathe(uint32_t color,uint64_t squares,uint32_t buffer[],uint16_t count,uint8_t delay){
	uint32_t new_color = 0;
	for(int i = 0; i < count;i++){
		uint32_t prev_color = new_color + 1;
			while(prev_color != new_color){
				prev_color = new_color;
				new_color = brighten(new_color,color);

				set_color_according_to_val(new_color,buffer,squares);
				ws2812_transmit(buffer,NUM_LEDS);
				reset();
				tim2_delay_ms(delay);
				}
		while(new_color > 0){
			new_color = dimm(new_color);
			set_color_according_to_val(new_color,buffer,squares);
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
			tim2_delay_ms(delay);
		}

	}
}
uint32_t dimm(uint32_t color){
	uint8_t r = (uint8_t)(color>>8);
	uint8_t g = (uint8_t)(color>>16);
	uint8_t b = (uint8_t)(color);

	uint8_t r_factor, b_factor, g_factor;

	r_factor = (r < 10) ? 1 : r/10;
	b_factor = (b < 10) ? 1 : b/10;
	g_factor = (g < 10) ? 1 : g/10;

	if(r > 3)
		r -= r_factor;

	if(g > 3)
		g -= g_factor;

	if(b > 3)
		b -= b_factor;

	if(r <= 5 && g <= 5 && b <= 5)
		r = g = b = 0;

	return ((uint32_t)(b | (g<<16) | (r<<8)));
}
uint32_t brighten(uint32_t color, uint32_t target_color) {
    uint8_t r = (uint8_t)(color >> 8);
    uint8_t g = (uint8_t)(color >> 16);
    uint8_t b = (uint8_t)(color);

    uint8_t target_r = (uint8_t)(target_color >> 8);
    uint8_t target_g = (uint8_t)(target_color >> 16);
    uint8_t target_b = (uint8_t)(target_color);

    uint8_t r_factor, b_factor, g_factor;

    // Increment proportional to how far from the target
    r_factor = (r < target_r) ? (target_r - r) / 10 : 0;
    g_factor = (g < target_g) ? (target_g - g) / 10 : 0;
    b_factor = (b < target_b) ? (target_b - b) / 10 : 0;

    if (r < target_r)
        r += r_factor;

    if (g < target_g)
        g += g_factor;

    if (b < target_b)
        b += b_factor;

    // Cap values to the target color
    if (r > target_r)
        r = target_r;
    if (g > target_g)
        g = target_g;
    if (b > target_b)
        b = target_b;

    return ((uint32_t)(b | (g << 16) | (r << 8)));
}

void set_color(uint32_t color,uint8_t index,uint32_t* buffer){
	buffer[index] = color;
}
void set_color_according_to_val(uint32_t color,uint32_t* buffer,uint64_t val){
	for(int i = 0; i < NUM_LEDS;i++){
		int col = i % 8;
		int row = i / 8;
		int corrected_index = row* 8 + 7-col;
		if(val & (1ULL<<i))
			buffer[corrected_index] = color;
		else
			buffer[corrected_index] = 0;
	}
}
void add_color_according_to_val(uint32_t color,uint32_t* buffer,uint64_t val){
	for(int i = 0; i < NUM_LEDS;i++){
		int col = i % 8;
		int row = i / 8;
		int corrected_index = row* 8 + 7-col;
			if(val & (1ULL<<i))buffer[corrected_index] = color;
	}
}
void set_all(uint32_t color,uint32_t* buffer){
	for(int i = 0;i < NUM_LEDS;i++){
		buffer[i] = color;
	}
}
void wake_up(uint32_t color,uint64_t squares,uint32_t buffer[],uint8_t delay){
	uint32_t new_color = 0;
	uint32_t prev_color = new_color + 1;
		while(prev_color != new_color){
			prev_color = new_color;
			new_color = brighten(new_color,color);

			set_color_according_to_val(new_color,buffer,squares);
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
			tim2_delay_ms(delay);
		}
}
void sleep(uint32_t color,uint64_t squares,uint32_t buffer[],uint8_t delay){
	while(color > 0){
		color = dimm(color);
		set_color_according_to_val(color,buffer,squares);
		ws2812_transmit(buffer,NUM_LEDS);
		reset();
		tim2_delay_ms(delay);
	}

}



