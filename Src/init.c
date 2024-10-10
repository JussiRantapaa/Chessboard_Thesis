#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "main.h"

void error_led_init(void){
	RCC->AHB1ENR |= GPIODEN;

	GPIOD->MODER |= 	(1U<<28);
	GPIOD->MODER &=~ 	(1U<<29);
}
void error_handler(uint8_t* error_code){
	if(*error_code)
		GPIOA->ODR |= ERROR_LED;
}
/* @brief Initialize a GPIO Pin to desired mode
 * @param 	GPIO_TypeDef* GPIOx = Port of the pin
 * @param	int pin = number of the pin
 * @param	enum GPIO_MODE = mode of the pin (INPUT,OUTPUT,AFR,ANALOG
 */
void GPIO_init(GPIO_TypeDef* GPIOx,uint8_t pin, enum GPIO_MODE mode,uint8_t* error_code){
	if(error_code == NULL){
		return;
	}
	if(GPIOx == NULL || GPIOx > GPIOI){
		*error_code = 1;
		return;
	}
	if(pin < 0 || pin > 15){
		*error_code = 2;
		return;
	}

	GPIO_clock_init((uint16_t)((uintptr_t)GPIOx & 0xFFFF));

	switch(mode){
		case INPUT:
			GPIOx->MODER &=~ (1U<<(pin * 2));
			GPIOx->MODER &=~ (1U<<(pin * 2 + 1));
			break;
		case OUTPUT:
			GPIOx->MODER |=  (1U<<(pin * 2));
			GPIOx->MODER &=~ (1U<<(pin * 2 + 1));
			break;
		case AFR:
			GPIOx->MODER &=~ (1U<<(pin * 2));
			GPIOx->MODER |=  (1U<<(pin * 2 + 1));
			break;
		case ANALOG:
			GPIOx->MODER |=  (1U<<(pin * 2));
			GPIOx->MODER |=	 (1U<<(pin * 2 + 1));
			break;
	}
}
void GPIO_clock_init(uint16_t GPIO_port_bits){
	if(GPIO_port_bits == 0){
		RCC->AHB1ENR |= GPIOAEN;
	}else{
		uint16_t enable_bit = GPIO_port_bits / 1024;
		RCC->AHB1ENR |= (1U<<enable_bit);
	}
}
/*
 * @brief Sets the alternate function mode
 *
* @param GPIO_TypeDef* GPIOx:	GPIO struct pointer (GPIOA - GPIOI)
 * @param int pin :				Pin number (0-15)
 * @param int mode:				AFR mode number (0-15)
 * @param int* error_code:		Pointer to the error_code integer
 */
void AFR_init(GPIO_TypeDef* GPIOx,uint8_t pin, uint8_t mode,uint8_t* error_code){
	uint8_t register_num = (pin < 8) ? 0 : 1;
	uint8_t register_offset = (pin % 8) * 4;

	if(pin < 0 || pin > 15 || mode < 0 || mode > 15){
		*error_code = 1;
		return;
	}
	GPIOx->AFR[register_num] |= (mode<<register_offset);
}
