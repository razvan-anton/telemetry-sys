#ifndef UART
#define UART

#include "stm32f1xx.h"

#define RX_BUF_SIZE 64 

extern volatile uint8_t rx_buffer[RX_BUF_SIZE];
extern volatile uint16_t rx_head;
extern volatile uint16_t rx_tail;

extern volatile uint16_t overflow_cnt;
extern volatile uint16_t error_cnt;




void USART2_IRQHandler();

void UART_Tx(uint8_t data);




#endif