#include "uart.h"

static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint16_t rx_head;
static volatile uint16_t rx_tail;

static volatile uint16_t overflow_cnt;
static volatile uint16_t error_cnt;

void USART2_IRQHandler()
{
    uint32_t status = USART2->SR;
    if(status & USART_SR_RXNE_Msk)
    {
        uint8_t byte = USART2->DR;
        uint16_t next_head = (rx_head + 1) & (RX_BUF_SIZE - 1);
        // bitwise & (N-1), when N is power of two, does mod N 

        // we have to read DR first so that RXNE is cleared.
        
        if(next_head != rx_tail)
        {
            rx_buffer[next_head]=byte;
            rx_head=next_head;
        }
        else
        {
            overflow_cnt++;
        }
    }    
    if(status & USART_SR_ORE_Msk)
    {
        (void)USART2->DR;
        // this just reads the DR without storing the var anywhere
        error_cnt++;
        // if we are here, we had corrupted data
        // This happens when data arrives in the DR reg faster than we process it.
        // ( or in other words if we didn't process data fast enough due to other interrupts)
        // when this happens this ISR is triggered and ORE is set and we ahve to clear it

        // the read ORE + read DR sequence is how to clear the ORE bit
    }
}

void USART2_init()
{
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

    NVIC_SetPriority(USART2_IRQn,1);
    NVIC_EnableIRQ(USART2_IRQn);
    // enable interrupts and set priority

    USART2->CR1 |= USART_CR1_UE_Msk;
    // enable UART

    USART2->CR1 |= USART_CR1_TE_Msk;
    USART2->CR1 |= USART_CR1_RE_Msk;
    USART2->CR1 |= USART_CR1_RXNEIE_Msk;
    // enable receiver, transmitter and interrupts for each byte received
}

void uart2_write_byte(uint8_t byte)
{
    while(!(USART2->SR & USART_SR_TXE_Msk));
    // TXE reg has default value of 1

    USART2->DR = byte;
    // after this write, TXE reg gets value 0. After data is moved to the shift register it is set.
}

void uart2_write_string(const char *str)
{
    if(!str) return;

    while(*str)
    {
        uart2_write_byte((uint8_t)*str);
        str++;
    }
}

bool uart2_rx_available(void)
{
    return (rx_head!=rx_tail);
}

bool uart2_read_byte(uint8_t *out_byte)
{
    if(!uart2_rx_available() || !out_byte)
        return false;

    (*out_byte) = rx_buffer[rx_tail];
    rx_tail = (rx_tail+1) & (RX_BUF_SIZE-1);

    return true;
}