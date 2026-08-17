#include "stm32f1xx.h"
#include "uart.h"
#include "scheduler.h"
#include "rcc.h"
#include "systick.h"

// st-flash --reset write build/app.bin 0x08000000
// picocom -b 115200 --imap lfcrlf /dev/ttyACM0

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

void BlinkLED(void)
{
    GPIOA->ODR ^= GPIO_ODR_ODR5;
}

void ReadUART_echo(void)
{
    uint8_t byte;
    while(uart2_read_byte(&byte))
    {
        uart2_write_byte(byte);
    }
}

void Heartbeat(void)
{
    const char * str = "Alive\r\n" ;
    uart2_write_string(str);
}

static Task task_arr[] = {
{ BlinkLED,         50,     0 },
{ ReadUART_echo,    200,    0 },
{ Heartbeat,        1000,   0 }
};

#define NUM_TASKS (sizeof(task_arr) / sizeof(task_arr[0]))


int main(void)
{
    RCC_init();

    gpio_init();

    USART2_init();

    SysTick_init();

    while (1) {
        for(uint32_t i=0; i < NUM_TASKS ; ++i)
        {
            if(delay_elapsed(task_arr[i].last_run_tick,task_arr[i].period_ms))
            {
                task_arr[i].last_run_tick+=task_arr[i].period_ms;
                task_arr[i].function();
            }
        }

    }
}

