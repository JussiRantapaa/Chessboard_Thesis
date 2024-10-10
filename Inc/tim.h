#ifndef TIM_H
#define TIM_H

#include <stdlib.h>
#include "stm32f4xx.h"

#define CR1_CEN		(1U<<0)
#define SR_UIF		(1U<<0)
#define TIM2EN		(1U<<0)
#define DIER_UIE	(1U<<0)
#define EGR_UG		(1U<<0)


void TIM2_init(void);
void tim2_delay_ms(uint32_t millis);

#endif

