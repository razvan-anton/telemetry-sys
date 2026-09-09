#ifndef I2C_H
#define I2C_H

#include "stm32f1xx.h"

typedef enum {
    I2C_OK           =  0,
    I2C_ERR_BUSY     = -1,
    I2C_ERR_NACK_ADDR= -2,
    I2C_ERR_NACK_DATA= -3,
    I2C_ERR_TIMEOUT  = -4,
    I2C_BAD_INPUT    = -5
} i2c_status_t;

void I2C1_init(void);

int8_t i2c_write_reg(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);

int8_t i2c_read_bytes(uint8_t slave_addr, uint8_t start_reg, uint8_t *data, uint16_t len);




#endif