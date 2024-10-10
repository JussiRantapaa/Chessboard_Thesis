
#ifndef WS2812B_H_
#define WS2812B_H_

#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "main.h"


#define NUM_LEDS  		64
#define NUM_BITS		24

#define HIGH_BIT		0x0F
#define LOW_BIT			0x03

// ALL COLORS ARE IN GRB ORDER SINCE THATS THE ORDER WS2812 EXPECTS IT
#define RED				0x00FF00
#define GREEN			0xFF0000
#define BLUE			0x0000FF
#define GREEN_LIGHT		0x420402
#define BEIGE 			0x5BDA32
#define YELLOW			0xF2F500
#define PINK			0x00B9FF
#define PURPLE			0x00FFFF
#define TURQOISE		0xF002F5
#define MINT			0xFA023F
#define ORANGE_LIGHT	0x94FF04
#define ORANGE			0x39A000
#define MAX				0xFFFFFF


uint32_t compute_color(uint8_t r, uint8_t g,uint8_t b);
void set_color(uint32_t color,uint8_t index,uint32_t* buffer);
void set_all(uint32_t color,uint32_t* buffer);
void set_color_according_to_val(uint32_t color,uint32_t* buffer,uint64_t val);
void add_color_according_to_val(uint32_t color,uint32_t* buffer,uint64_t val);
void test(uint32_t* buffer,uint32_t size);
uint32_t dimm(uint32_t color);
void breathe(uint32_t color,uint64_t squares,uint32_t buffer[],uint16_t count,uint8_t delay);
void wake_up(uint32_t color,uint64_t squares,uint32_t buffer[],uint8_t delay);
void sleep(uint32_t color,uint64_t squares,uint32_t buffer[],uint8_t delay);
uint32_t brighten(uint32_t color, uint32_t target_color);
extern const uint8_t gamma_table[];
extern uint8_t RESET_BUFFER[420];
#endif /* WS2812B_H_ */
