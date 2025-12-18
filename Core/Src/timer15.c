#include "timer15.h"

TIM_HandleTypeDef htimer15;
/*
Timer 15 connected to apb2 
the apb2 bus is set in the clock config to 80Mhz 
i need a on1 second delay

*/

void MX_Timer15_Init(void)
{
    __HAL_RCC_TIM15_CLK_ENABLE();

    htimer15.Instance = TIM15;
    htimer15.Init.Prescaler = 79;     // 1 MHz -> 1 tick = 1 µs
    htimer15.Init.Period = 0xFFFF;    // Max 65535 µs = 65 ms per overflow
    htimer15.Init.CounterMode = TIM_COUNTERMODE_UP;
    htimer15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htimer15.Init.RepetitionCounter = 0;
    htimer15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_TIM_Base_Init(&htimer15);
    HAL_TIM_Base_Start(&htimer15);
}

void Delay_us(uint16_t us)
{
    uint16_t start = __HAL_TIM_GET_COUNTER(&htimer15);
    while((uint16_t)(__HAL_TIM_GET_COUNTER(&htimer15) - start) < us);
}

void Delay(uint16_t ms)
{
    while(ms--) Delay_us(1000);
}


