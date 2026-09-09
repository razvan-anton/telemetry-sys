#ifndef UART_H
#define UART_H

#include "stm32f1xx.h"
#include <stdbool.h>
#include <string.h>


#define RX_BUF_SIZE 128
#define TX_BUF_SIZE 128


void USART3_IRQHandler();

void USART3_init();

// Transmission (TX)
bool uart3_write_byte(uint8_t byte);
void uart3_write_string(const char *str);

// eception (RX - Non-blocking)
bool uart3_rx_available(void);
bool uart3_read_byte(uint8_t *out_byte);

bool uart3_tx_empty(void);
bool uart3_tx_full(void);




#endif