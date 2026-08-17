#include "stm32f1xx.h"
#include "uart.h"
#include "scheduler.h"
#include "rcc.h"
#include "systick.h"

static void gpio_init(void)
{
    GPIOA->CRL &= ~GPIO_CRL_MODE5_Msk;
    GPIOA->CRL &= ~GPIO_CRL_CNF5_Msk;
    GPIOA->CRL |= GPIO_CRL_MODE5_1;
    //enable push pull output and clock speed of 2MHz for Port A
}

void SystemInit(void)
{
    /* Clock / system initialization stub */
}

int main(void)
{
    RCC_init();

    gpio_init();

    USART2_init();

    SysTick_init();

    uint32_t last_time=0;

    while (1) {

        if(delay_elapsed(last_time,500))
        {
            GPIOA->ODR ^= GPIO_ODR_ODR5;
            // toggle pin 5 ( PA5 is for the User LED )

            last_time+=500; // to force it every 500ms without any drifts
        }


    }
}

