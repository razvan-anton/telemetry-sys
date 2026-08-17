#include "rcc.h"

void RCC_init()
{
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

    RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
    //enable clock for Port A
}

