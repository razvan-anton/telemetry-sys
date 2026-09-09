#ifndef MPU_H
#define MPU_H

#include "i2c.h"
#include <stdint.h>


// MPU6050 register map

#define MPU6050_I2C_ADDR            0x68

/* Configuration Registers */
#define MPU6050_REG_SMPLRT_DIV      0x19
#define MPU6050_REG_CONFIG          0x1A
#define MPU6050_REG_GYRO_CONFIG     0x1B
#define MPU6050_REG_ACCEL_CONFIG    0x1C
#define MPU6050_SENSORS_VAL         0x00

/* Data Output Base Registers */
#define MPU6050_REG_ACCEL_XOUT_H    0x3B
#define MPU6050_REG_TEMP_OUT_H      0x41
#define MPU6050_REG_GYRO_XOUT_H     0x43

/* Power and Reset Management */
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_PWR_MGMT_2      0x6C
#define MPU6050_PWR_RESET_BIT       0x08

/* Identity Register & Expected Response */
#define MPU6050_REG_WHO_AM_I        0x75
#define MPU6050_WHO_AM_I_VAL        0x68

/* Bitmasms / Register Vals */
#define MPU6050_PWR_RESET           0x80  /* software reset */
#define MPU6050_CLK_PLL_X           0x01  /* Wake Sensor + X-axis PLL */
#define MPU6050_DLPF_CFG_4          0x04  /* Change sensitivity settings */

/* Sensitivity Scale Factors */
#define MPU6050_ACCEL_SENS_2G       16384.0f  /* LSB/g at default config */
#define MPU6050_GYRO_SENS_250DPS    131.0f    /* LSB/(deg/s) at default config */

#define MAX_RESET_RETRIES           50

typedef enum {
    MPU_OK          =    0,
    MPU_BUSY        =    1,
    MPU_INIT_ERR    =   -1,
    MPU_I2C_ERR     =   -2,
    MPU_RST_ERR     =   -3,
    MPU_UNINIT      =   -4
} mpu_status_t;

typedef enum{
    MPU_STATE_UNINITIALISED,
    MPU_STATE_RESET_WAIT,
    MPU_STATE_CONFIGURING,
    MPU_STATE_READY,
    MPU_STATE_ERROR,
} mpu_state_t;

typedef struct {
    /* Scaled sensor engineering units */
    float ax, ay, az;     /* In g (gravity) */
    float gx, gy, gz;     /* In deg/s (DPS) */
    float temp_c;         /* In degrees Celsius */
} mpu_data_t;

mpu_status_t mpu_check_reset(void);
mpu_status_t mpu_trigger_reset(void);
// reset might take even 100ms to complete, so we poll the register in the FSM every time the function from main is called
// so as to keep our task scheduler cooperative

mpu_status_t mpu_init(void);

mpu_status_t mpu_process(mpu_data_t * out_data); // main FSM logic

mpu_status_t mpu_read_data(mpu_data_t *data);


#endif