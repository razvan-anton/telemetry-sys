#include "uart.h"

static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint16_t rx_head;
static volatile uint16_t rx_tail;

static volatile uint8_t tx_buffer[TX_BUF_SIZE];
static volatile uint16_t tx_head;
static volatile uint16_t tx_tail;

static volatile uint16_t rx_overflow_cnt;
static volatile uint16_t rx_error_cnt;
static volatile uint16_t tx_overflow_cnt;
static volatile uint16_t tx_error_cnt;

void USART3_init()
{
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN_Msk;
    //enable USART3 clock

    GPIOB->CRH |= GPIO_CRH_MODE10_Msk;
    GPIOB->CRH &= ~GPIO_CRH_CNF10_0;
    GPIOB->CRH |= GPIO_CRH_CNF10_1;
    // configure pin PB10 for TX; 50 MHz output mode, Alternate function output Push-pull

    GPIOB->CRH &= ~GPIO_CRH_MODE11_Msk;
    GPIOB->CRH &= ~GPIO_CRH_CNF11_Msk;
    GPIOB->CRH |= GPIO_CRH_CNF11_0;
    // configure PB11 for RX; input mode, floating input

    USART3->BRR = 0x138;
    // the USARTDIV value for a 115200 baud rate at a 36Mhz clock

    NVIC_SetPriority(USART3_IRQn,1);
    NVIC_EnableIRQ(USART3_IRQn);
    // enable interrupts and set priority

    USART3->CR1 |= USART_CR1_UE_Msk;
    // enable UART

    USART3->CR1 |= USART_CR1_TE_Msk;
    USART3->CR1 |= USART_CR1_RE_Msk;
    USART3->CR1 |= USART_CR1_RXNEIE_Msk;
    // enable receiver, transmitter and interrupts for each byte received
}

void USART3_IRQHandler()
{
    uint32_t status = USART3->SR;

    if(status & USART_SR_ORE_Msk)
    {
        (void)USART3->DR;
        // this just reads the DR without storing the var anywhere
        rx_error_cnt++;
        // if we are here, we had corrupted data
        // This happens when data arrives in the DR reg faster than we process it.
        // ( or in other words if we didn't process data fast enough due to other interrupts)
        // when this happens this ISR is triggered and ORE is set and we ahve to clear it

        // the read ORE + read DR sequence is how to clear the ORE bit
    }
    else if(status & USART_SR_RXNE_Msk)
    {
        //only read data if no errors
        uint8_t byte = USART3->DR;
        rx_buffer[rx_head]=byte;
        uint16_t next_head = (rx_head + 1) & (RX_BUF_SIZE - 1);
        // bitwise & (N-1), when N is power of two, does mod N 

        // we have to read DR first so that RXNE is cleared.
        
        if(next_head != rx_tail)
        { 
            rx_head=next_head;
        }
        else
        {
            rx_overflow_cnt++;
        }
    }    

    if((status & USART_SR_TXE_Msk) && (USART3->CR1 & USART_CR1_TXEIE_Msk))
    // if TX interrupts are enabled AND we can write to the DR reg
    {
        if(!uart3_tx_empty())
        {
            uint8_t byte = tx_buffer[tx_tail];
            tx_tail = (tx_tail+1) & (TX_BUF_SIZE - 1);

            USART3->DR = byte;
            // after this write, TXE reg gets value 0. After data is moved to the shift register it is set.
        }
            else
        {
            USART3->CR1 &= ~USART_CR1_TXEIE_Msk;
            // disable interrupts; we have sent the full message
        }
    }


}



bool uart3_write_byte(uint8_t byte)
{
    if(uart3_tx_full())
    {
        tx_overflow_cnt++;
        return false;
    }

    tx_buffer[tx_head]=byte;
    tx_head = (tx_head + 1) & (TX_BUF_SIZE -1 );

    NVIC_DisableIRQ(USART3_IRQn);

    USART3->CR1 |= USART_CR1_TXEIE_Msk;
    // allow ISR to trigger

    NVIC_EnableIRQ(USART3_IRQn);
    // this prevents races: ex race right before the reg update, TXEIE set to 0, then set to 1 again here

    return true;
}

void uart3_write_string(const char *str)
{
    if(!str) return;

    while(*str)
    {
        uart3_write_byte((*str));
        str++;
    }
}

bool uart3_rx_available(void)
{
    return (rx_head!=rx_tail);
}

bool uart3_tx_empty(void)
{
    return (tx_head==tx_tail);
}

bool uart3_tx_full(void)
{
    return (((tx_head+1) & (TX_BUF_SIZE-1)) == tx_tail);
}

bool uart3_read_byte(uint8_t *out_byte)
{
    if(!uart3_rx_available() || !out_byte)
        return false;

    (*out_byte) = rx_buffer[rx_tail];
    rx_tail = (rx_tail+1) & (RX_BUF_SIZE-1);

    return true;
}