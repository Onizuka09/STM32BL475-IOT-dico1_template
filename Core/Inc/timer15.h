#ifndef  __TIMER_15_H__ 
#define  __TIMER_15_H__
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"

extern TIM_HandleTypeDef htimer15;


void MX_Timer15_Init(); 
void Delay(uint16_t delay); 


#endif 