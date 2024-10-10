#include <stdlib.h>
#include "74HCT597.h"
#include "main.h"
#include "spi.h"
#include "stm32f4xx.h"
#include "tim.h"
#include <math.h>

void shift_init(void){
	uint8_t error_code = 0;
	// Init BTN, MR, PL and STCP pins
	GPIO_init(GPIOA,1,OUTPUT,&error_code);
	GPIO_init(GPIOA,2,OUTPUT,&error_code);
	GPIO_init(GPIOA,3,OUTPUT,&error_code);

	GPIOA->ODR &=~ SHIFT_MR;

	tim2_delay_ms(1);
	GPIOA->ODR |= SHIFT_MR;

}
void shift_load(void){
	// Load flip-flops
	GPIOA->ODR |= SHIFT_STCP;
	tim2_delay_ms(1);
	GPIOA->ODR &=~ SHIFT_STCP;

	// Set parellel load low to load the data from inputs
	GPIOA->ODR &=~ SHIFT_PL;
	// Delay
	tim2_delay_ms(1);
	// Set parellel load high to stop loading
	GPIOA->ODR |= SHIFT_PL;
}
uint8_t shift_read_8bit(void){
	shift_load();
	return SPI2_receive_8bit();
}
uint16_t shift_read_16bit(void){
	shift_load();
	return SPI2_receive_16bit();
}
uint64_t shift_read_64bit(void){
	shift_load();
	uint64_t data = 0;
	for(int i = 0; i < 8;i++){
		uint8_t read_val = SPI2_receive_8bit();
		read_val = ~read_val;
		data += ((uint64_t)read_val << (i*8));
	}
	return (data);
}


