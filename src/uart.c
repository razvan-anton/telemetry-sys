#include "uart.h"

volatile uint8_t rx_buffer[RX_BUF_SIZE];
volatile uint16_t rx_head;
volatile uint16_t rx_tail;

volatile uint16_t overflow_cnt;
volatile uint16_t error_cnt;

void UART_Tx(uint8_t data)
{
    while(!(USART2->SR & USART_SR_TXE_Msk));
    // TXE reg has default value of 1

    USART2->DR = data;
    // after this write, TXE reg gets value 0. After data is moved to the shift register it is set.
}

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