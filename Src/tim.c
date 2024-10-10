#include <stdlib.h>
#include "tim.h"

void TIM2_init(void){
	//Enable clock access
	RCC->APB1ENR |= TIM2EN;
	// Disable timer
	TIM2->CR1 &=~ CR1_CEN;

	// Set timer to 1 Hz: 16 000 000 / 1600 = 10 000, 10 000 / 10 000 = 1 Hz
	TIM2->PSC = 15;
	TIM2->ARR = 1000 - 1;
	TIM2->CNT = 0;


	// Enable Timer
	TIM2->CR1 |= CR1_CEN;
}
void tim2_delay_ms(uint32_t millis){
	TIM2->CNT = 0;
	for(int i = 0;i < millis;i++){
		while(!(TIM2->SR & SR_UIF)){}
		TIM2->SR &=~ SR_UIF;
	}
}


