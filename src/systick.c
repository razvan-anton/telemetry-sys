#include "systick.h"


volatile uint32_t system_ticks; // expressed in ms

void SysTick_Handler()
{
    ++system_ticks;
}

uint8_t delay_elapsed(uint32_t start_ticks, uint32_t delay)
{
    return ((system_ticks - start_ticks) >= delay);
}

void SysTick_init()
{
    SysTick->LOAD=71999u; //count this number of clock cycles (equivallent to 1 ms)
    SysTick->VAL=0u;    // reset current value
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
    // start SysTick and make it trigger an interrupt ( to call SysTick_Handler )
}