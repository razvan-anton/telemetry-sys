#include "stm32f1xx.h"

void SystemInit(void)
{
    /* Clock / system initialization stub */
}

int main(void)
{
    RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
    //enable clock for Port A

    GPIOA->CRL &= ~GPIO_CRL_MODE5_Msk;
    GPIOA->CRL &= ~GPIO_CRL_CNF5_Msk;
    GPIOA->CRL |= GPIO_CRL_MODE5_1;
    //enable push pull output and clock speed of 2MHz for Port A

    SysTick->LOAD=7999999u; //count this number of clock cycles (equivallent to 1 second)
    SysTick->VAL=0u;    // reset current value
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    while (1) {

        while( !(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) );
        // poll CTRL flag until it has counted

        GPIOA->ODR ^= GPIO_ODR_ODR5;
        // toggle pin 5 ( PA5 is for the User LED )

    }
}