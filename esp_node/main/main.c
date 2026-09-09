#include <stdio.h>
#include "driver/uart.h"

void app_main(void)
{
    // config uart
    uart_config_t uart_config;
    uart_config.baud_rate = 115200; 
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity=UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    // this leaves the first UART port open, 
    uart_param_config(UART_NUM_1,&uart_config);

    // set pins 17 and 16 for TX, RX on the UART1 controller
    uart_set_pin(UART_NUM_1, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM_1,1024,0,0,NULL,0);

    

}
