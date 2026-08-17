#ifndef UART_H
#define UART_H

#include "stm32f1xx.h"
#include <stdbool.h>


#define RX_BUF_SIZE 64 


void USART2_IRQHandler();

void USART2_init();

// Transmission (TX)
void uart2_write_byte(uint8_t byte);
void uart2_write_string(const char *str);

// eception (RX - Non-blocking)
bool uart2_rx_available(void);
bool uart2_read_byte(uint8_t *out_byte);




#endif