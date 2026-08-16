#include "stm32f1xx.h"

volatile uint32_t system_ticks; // expressed in ms

void SysTick_Handler()
{
    ++system_ticks;
}

uint8_t delay_elapsed(uint32_t start_ticks, uint32_t delay)
{
    return ((system_ticks - start_ticks) >= delay);
}

void UART_Tx(uint8_t data)
{
    while(!(USART2->SR & USART_SR_TXE_Msk));
    // TXE reg has default value of 1

    USART2->DR = data;
    // after this write, TXE reg gets value 0. After data is moved to the shift register it is set.
}

// void USART2_IRQHandler()
// {
    
// }

void SystemInit(void)
{
    /* Clock / system initialization stub */
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2;
    //( Flash reads are slower than the max speed clock so for when reading flash
    // we add 2 wait states so it matches the Clock)

    FLASH->ACR |= FLASH_ACR_PRFTBE;
    // enable prefetch buffer

    RCC->CR |= RCC_CR_HSEON_Msk;
    while( !(RCC->CR & RCC_CR_HSERDY) );
    // enable HSE and wait for it to stabilise

    RCC->CR &= ~RCC_CR_PLLON;
    while( RCC->CR & RCC_CR_PLLRDY );
    // disable PLL so we can modify it

    RCC->CFGR &= ~RCC_CFGR_PPRE2_Msk;
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

    RCC->CFGR &= ~RCC_CFGR_PPRE1_Msk;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    // configure max safe speeds for APB1, APB2 and AHB

    RCC->CFGR |= RCC_CFGR_PLLSRC;
    // make HSE the entry of PLL

    RCC->CFGR &= ~RCC_CFGR_PLLMULL_Msk;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;
    // multiply HSE by 9, so 8*9 = 72 MHz clock ( max speed )

    RCC->CR |= RCC_CR_PLLON;
    while( !(RCC->CR & RCC_CR_PLLRDY) );
    // enable PLL again

    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while( !(RCC->CFGR & RCC_CFGR_SWS_PLL));
    // use PLL as System Clock

}

int main(void)
{
    RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
    //enable clock for Port A

    GPIOA->CRL &= ~GPIO_CRL_MODE5_Msk;
    GPIOA->CRL &= ~GPIO_CRL_CNF5_Msk;
    GPIOA->CRL |= GPIO_CRL_MODE5_1;
    //enable push pull output and clock speed of 2MHz for Port A

    RCC->APB1ENR |= RCC_APB1ENR_USART2EN_Msk;
    //enable USART2 clock

    GPIOA->CRL |= GPIO_CRL_MODE2_Msk;
    GPIOA->CRL &= ~GPIO_CRL_CNF2_0;
    GPIOA->CRL |= GPIO_CRL_CNF2_1;
    // configure pin PA2 for TX; 50 MHz output mode, Alternate function output Push-pull

    GPIOA->CRL &= ~GPIO_CRL_MODE3_Msk;
    GPIOA->CRL &= ~GPIO_CRL_CNF3_Msk;
    GPIOA->CRL |= GPIO_CRL_CNF3_0;
    // configure PA3 for RX; input mode, floating input

    USART2->BRR = 0x138;
    // the USARTDIV value for a 115200 baud rate at a 36Mhz clock

    // NVIC_SetPriority(USART2_IRQn,1);
    // NVIC_EnableIRQ(USART2_IRQn);
    // // enable interrupts and set priority

    USART2->CR1 |= USART_CR1_UE_Msk;
    // enable UART

    USART2->CR1 |= USART_CR1_TE_Msk;
    USART2->CR1 |= USART_CR1_RE_Msk;
    USART2->CR1 |= USART_CR1_RXNEIE_Msk;
    // enable receiver, transmitter and interrupts for each byte received

    SysTick->LOAD=71999u; //count this number of clock cycles (equivallent to 1 ms)
    SysTick->VAL=0u;    // reset current value
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
    // start SysTick and make it trigger an interrupt ( to call SysTick_Handler )

    uint32_t last_time=0;

    while (1) {

        if(delay_elapsed(last_time,500))
        {
            GPIOA->ODR ^= GPIO_ODR_ODR5;
            // toggle pin 5 ( PA5 is for the User LED )

            UART_Tx('H');
            UART_Tx('E');
            UART_Tx('L');
            UART_Tx('L');
            UART_Tx('O');
            UART_Tx('\r');
            UART_Tx('\n');


            last_time+=500; // to force it every 500ms without any drifts
        }


    }
}

