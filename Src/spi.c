
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "spi.h"
#include "ws2812b.h"
#include "main.h"
#include "tim.h"

void spi1_init(void){
	uint8_t error_code = 0;
	// Init SPI pins PA5 SCK, PA7 MOSI
	GPIO_init(GPIOA,5,AFR,&error_code);
	GPIO_init(GPIOA,7,AFR,&error_code);

	GPIOA->OSPEEDR |= (1U<<14);
	GPIOA->OSPEEDR |= (1U<<15);
	GPIOA->OSPEEDR |= (1U<<10);
	GPIOA->OSPEEDR |= (1U<<11);

	AFR_init(GPIOA,5,5,&error_code);
	AFR_init(GPIOA,7,5,&error_code);

	// Enable SPI1
	RCC->APB2ENR |= SPI1EN;
	// Disable SPI while we configure it
	SPI1->CR1 &=~ CR1_SPE;
	// Set master mode, clock polarity, MSB first, dataformat to 8 bits
	SPI1->CR1 |= CR1_MSTR;
	SPI1->CR1 &=~ CR1_CPHA;
	SPI1->CR1 &=~ CR1_CPOL;
	SPI1->CR1 &=~ CR1_LSB;
	SPI1->CR1 &=~ CR1_DFF;

	SPI1->CR2 |= (1U<<2);
	// Set baudrate to fClock/2
	SPI1->CR1 &=~ (1U<<3);
	SPI1->CR1 &=~ (1U<<4);
	SPI1->CR1 &=~ (1U<<5);

	SPI1->CR1 |= CR1_SPE;
}
void spi1_transmit(uint8_t* data,uint32_t size){
	uint32_t i = 0;
	while(i < size){
		while(!(SPI1->SR & SR_TXE)){}
		*((volatile uint8_t*) &(SPI1->DR)) = data[i];
		i++;
	}
	while(!(SPI1->SR & SR_TXE)){}
	while(SPI1->SR & SR_BSY){}
}
void ws2812_transmit(uint32_t* data,uint32_t size){
	uint32_t i = 0;
	while(i < size){
		for(int bit = 23;bit >= 0;bit--){
			while(!(SPI1->SR & SR_TXE)){}
			if(data[i] & (1U<<bit))
				*((volatile uint8_t*) &(SPI1->DR)) = (uint8_t)HIGH_BIT;
			else
				*((volatile uint8_t*) &(SPI1->DR)) = (uint8_t)LOW_BIT;
		}
		i++;
	}
	while(!(SPI1->SR & SR_TXE)){}
	while(SPI1->SR & SR_BSY){}

}
void ws2812_delayed_transmit(uint32_t* data,uint32_t size,uint32_t delay){
	uint32_t i = 0;

	while(i < size){
		for(int bit = 23;bit >= 0;bit--){
			while(!(SPI1->SR & SR_TXE)){}
			if(data[i] & (1U<<bit))
				*((volatile uint8_t*) &(SPI1->DR)) = (uint8_t)HIGH_BIT;
			else
				*((volatile uint8_t*) &(SPI1->DR)) = (uint8_t)LOW_BIT;
		}
		i++;
		if(i == NUM_LEDS){
			tim2_delay_ms(100);
		}
		if(i % NUM_LEDS == 0){
			reset();
			tim2_delay_ms(delay);
		}
			//
	}
	while(!(SPI1->SR & SR_TXE)){}
	while(SPI1->SR & SR_BSY){}

}
void spi1_transmit_24bits(uint32_t data,uint32_t size){
	uint32_t i = 0;

	while(i < size){
		while(!(SPI1->SR & SR_TXE)){}
			SPI1->DR = (uint8_t)data;

			while(!(SPI1->SR & SR_TXE)){}
			SPI1->DR = (uint8_t)(data>>8);

			while(!(SPI1->SR & SR_TXE)){}
			SPI1->DR = (uint8_t)(data>>16);
		i++;
	}

	while(!(SPI1->SR & SR_TXE)){}
	// Wait till BUSY flag is cleared

	while(SPI1->SR & SR_BSY){}
}
void reset(void){
	spi1_transmit(RESET_BUFFER,sizeof(RESET_BUFFER)/sizeof(uint8_t));
}
void SPI2_init_8bit(void){
	uint8_t error_code = 0;
	// Init SPI pin PB13 SCK, PB14 MISO PB15 CS
	GPIO_init(GPIOB,13,AFR,&error_code);
	GPIO_init(GPIOB,14,AFR,&error_code);
	GPIO_init(GPIOB,15,OUTPUT,&error_code);

	AFR_init(GPIOB,13,5,&error_code);
	AFR_init(GPIOB,14,5,&error_code);

	// Enable SPI2
	RCC->APB1ENR |= SPI2EN;
	// Disable SPI while we configure it
	SPI2->CR1 &=~ CR1_SPE;
	// Set master mode, clock polarity, MSB first, dataformat to 8 bits
	SPI2->CR1 |= CR1_MSTR;
	SPI2->CR1 &=~ CR1_CPHA;
	SPI2->CR1 &=~ CR1_CPOL;
	SPI2->CR1 &=~ CR1_LSB;
	SPI2->CR1 &=~ CR1_DFF;

	// Receive only
	SPI2->CR1 &=~ CR1_RXONLY;

	SPI2->CR2 |= CR2_SSOE;
	// Set baudrate to fClock/2
	SPI2->CR1 &=~ (1U<<3);
	SPI2->CR1 &=~ (1U<<4);
	SPI2->CR1 &=~ (1U<<5);

	SPI2->CR1 |= CR1_SPE;
}
void SPI2_init_16bit(void){
	uint8_t error_code = 0;
	// Init SPI pin PB13 SCK, PB14 MISO PB15 CS
	GPIO_init(GPIOB,13,AFR,&error_code);
	GPIO_init(GPIOB,14,AFR,&error_code);
	GPIO_init(GPIOB,15,OUTPUT,&error_code);

	AFR_init(GPIOB,13,5,&error_code);
	AFR_init(GPIOB,14,5,&error_code);

	// Enable SPI2
	RCC->APB1ENR |= SPI2EN;
	// Disable SPI while we configure it
	SPI2->CR1 &=~ CR1_SPE;
	// Set master mode, clock polarity, MSB first, dataformat to 16 bits
	SPI2->CR1 |= CR1_MSTR;
	SPI2->CR1 &=~ CR1_CPHA;
	SPI2->CR1 &=~ CR1_CPOL;
	SPI2->CR1 &=~ CR1_LSB;
	SPI2->CR1 |= CR1_DFF;

	// Receive only
	SPI2->CR1 &=~ CR1_RXONLY;

	SPI2->CR2 |= CR2_SSOE;
	// Set baudrate to fClock/2
	SPI2->CR1 &=~ (1U<<3);
	SPI2->CR1 &=~ (1U<<4);
	SPI2->CR1 &=~ (1U<<5);

	SPI2->CR1 |= CR1_SPE;
}
uint8_t SPI2_receive_8bit(void){
	cs_enable();
	SPI2->DR = 0;

	while(!(SPI2->SR & SR_RXNE)){

	}
	uint8_t data = SPI2->DR;
	cs_disable();

	return data;
 }
uint16_t SPI2_receive_16bit(void){
	cs_enable();
	SPI2->DR = 0;

	while(!(SPI2->SR & SR_RXNE)){
		GPIOD->ODR |= LED2;
	}
	uint16_t data = SPI2->DR;
	cs_disable();
	return data;
 }
void cs_enable(void){
	 GPIOB->ODR &=~ SPI2_CS;
}
void cs_disable(void){
	 GPIOB->ODR |= SPI2_CS;
}

