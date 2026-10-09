#include "main.h"

static QueueHandle_t telemetry_queue;
static int error_cnt;
int dropped_frames_cnt;
static const char *TAG = "UART_TELEMETRY";

void SendData(void)
{
    mpu_data_t rx_data={};
    int status=0;
    while(1)
    {
        if(xQueueReceive(&telemetry_queue,&rx_data,0) == pdPASS)
        {
            // if item successfully received
            ESP_LOGI(TAG, "IMU [g] Accel: (%.2f, %.2f, %.2f) | Gyro [dps]: (%.2f, %.2f, %.2f) | Temp: %.1f C",
            rx_data.ax, rx_data.ay, rx_data.az,
            rx_data.gx, rx_data.gy, rx_data.gz,
            rx_data.temp_c);
        }
    }
}

void ReadData(void)
{
    int status=0;
    int bytes_read=0;
    mpu_data_t tx_data={};
    while(1)
    {
        // block until data comes
        bytes_read = uart_read_bytes(UART_NUM_1, &tx_data, sizeof(mpu_data_t), portMAX_DELAY);
        if(bytes_read!=sizeof(mpu_data_t))
        {
            error_cnt++;
        }
        else
        {
            if( !xQueueSend(&telemetry_queue,&tx_data,0) == pdPASS)
            {
                dropped_frames_cnt++;
            }
        }

    }
}

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

    // create FreeRTOS queue
    telemetry_queue = xQueueCreate(10, sizeof(mpu_data_t));
    // we use a queue to share data between tasks

    assert(telemetry_queue!=NULL); //if not enough heap memory

    // create tasks, one for each core
    int status = xTaskCreatePinnedToCore(ReadData, "task1_read", 4096, NULL, 10, NULL, 0);
    assert(status == 1);
    status = xTaskCreatePinnedToCore(SendData, "task2_send", 4096, NULL, 5, NULL, 1);
    assert(status == 1);

}
