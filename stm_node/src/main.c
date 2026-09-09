#include "stm32f1xx.h"
#include "uart.h"
#include "scheduler.h"
#include "rcc.h"
#include "systick.h"
#include "i2c.h"
#include "mpu.h"
#include <stdio.h>

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
    while(uart3_read_byte(&byte))
    {
        uart3_write_byte(byte);
    }
}

void Heartbeat(void)
{
    const char * str = "Alive\r\n" ;
    uart3_write_string(str);
}

void ReadSensor_I2C(void)
{
    static mpu_status_t last_status = MPU_UNINIT;
    // use a rising edge so if we get an error, terminal isn't flooded, unless error type changes
    mpu_data_t data={};

    char tx_buf[96];

    mpu_status_t curr_status = mpu_process(&data);


    // DEBUG
    if (curr_status != last_status) {
    uart3_write_string("FSM State: ");
    uart3_write_byte('0' + curr_status);
    uart3_write_string("\r\n");
    }

    if(curr_status) //if error
    {
        if (last_status != curr_status) {
            switch (curr_status) {
                case MPU_OK:       uart3_write_string("0 (MPU_OK)\r\n");       break;
                case MPU_BUSY:     uart3_write_string("1 (MPU_BUSY)\r\n");     break;
                case MPU_INIT_ERR: uart3_write_string("-1 (INIT_ERR)\r\n");    break;
                case MPU_I2C_ERR:  uart3_write_string("-2 (I2C_ERR)\r\n");     break;
                case MPU_RST_ERR:  uart3_write_string("-3 (RST_ERR)\r\n");     break;
                case MPU_UNINIT:   uart3_write_string("-4 (MPU_UNINIT)\r\n");  break;
                default:           uart3_write_string("UNKNOWN ERR\r\n");      break;
            }
            last_status = curr_status;
        }
    }
    else
    {
        // if successful read, send it to UART
        // trick to have floats printed as ints, for speed,
        int len = snprintf(tx_buf, sizeof(tx_buf),
                           "ACC:[%d.%02d, %d.%02d, %d.%02d] | GYR:[%d.%02d, %d.%02d, %d.%02d] | T:%d.%02d C\r\n",
                           (int)data.ax, (int)(__builtin_abs((int)(data.ax * 100)) % 100),
                           (int)data.ay, (int)(__builtin_abs((int)(data.ay * 100)) % 100),
                           (int)data.az, (int)(__builtin_abs((int)(data.az * 100)) % 100),
                           (int)data.gx, (int)(__builtin_abs((int)(data.gx * 100)) % 100),
                           (int)data.gy, (int)(__builtin_abs((int)(data.gy * 100)) % 100),
                           (int)data.gz, (int)(__builtin_abs((int)(data.gz * 100)) % 100),
                           (int)data.temp_c, (int)(__builtin_abs((int)(data.temp_c * 100)) % 100));

        if (len > 0) {
            uart3_write_string(tx_buf);
        }

    }


}

static Task task_arr[] = {
{ BlinkLED,         50,     0 },
{ ReadUART_echo,    200,    0 },
{ Heartbeat,        2000,   0 }
//{ ReadSensor_I2C,   500,    0 }
};

#define NUM_TASKS (sizeof(task_arr) / sizeof(task_arr[0]))


int main(void)
{
    RCC_init();

    gpio_init();

    USART3_init();

    I2C1_init();

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

