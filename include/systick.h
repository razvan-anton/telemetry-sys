#ifndef systick_H
#define systick_H

#include "stm32f1xx.h"

void SysTick_Handler();

uint8_t delay_elapsed(uint32_t start_ticks, uint32_t delay);

void SysTick_init();



#endif