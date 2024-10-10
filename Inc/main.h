#include "stm32f4xx.h"

#ifndef MAIN_H_
#define MAIN_H_

#define GPIOAEN		(1U<<0)
#define GPIOBEN		(1U<<1)
#define GPIOCEN		(1U<<2)
#define GPIODEN		(1U<<3)
#define GPIOEEN		(1U<<4)
#define GPIOFEN		(1U<<5)
#define GPIOGEN		(1U<<6)
#define GPIOHEN		(1U<<7)
#define GPIOIEN		(1U<<8)
#define GPIOJEN		(1U<<9)
#define GPIOKEN		(1U<<10)

#define LED1		(1U<<12)
#define LED2		(1U<<13)
#define ERROR_LED	(1U<<14)
#define LED3		(1U<<15)

#define BTN			(1U<<0)

enum GPIO_MODE {
	INPUT,
	OUTPUT,
	AFR,
	ANALOG
};
void GPIO_init(GPIO_TypeDef* GPIOx,uint8_t pin, enum GPIO_MODE mode,uint8_t* error_code);
void AFR_init(GPIO_TypeDef* GPIOx,uint8_t pin, uint8_t mode,uint8_t* error_code);
void GPIO_clock_init(uint16_t GPIO_port_bits);
void error_led_init(void);
void error_handler(uint8_t* error_code);

void tim2_init(void);
#endif /* MAIN_H_ */
