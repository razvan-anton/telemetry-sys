#include "mpu.h"

mpu_state_t MPU_State;
mpu_status_t mpu_error_status = MPU_UNINIT;

mpu_status_t mpu_trigger_reset(void)
{
    // reset sensor, good practice before configuring it
    if(i2c_write_reg(MPU6050_I2C_ADDR, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR_RESET))
    {
        return MPU_I2C_ERR;
    }

    return MPU_OK;
}

mpu_status_t mpu_check_reset(void)
{
    static uint8_t reset_cnt=0;
    // wait for reset to be done
    uint8_t reg_val=0;
    if(reset_cnt++ > MAX_RESET_RETRIES)
    {
        reset_cnt=0;
        return MPU_RST_ERR;
    }
    if(i2c_read_bytes(MPU6050_I2C_ADDR, MPU6050_REG_PWR_MGMT_1, &reg_val,1))
    {
        reset_cnt=0;
        return MPU_I2C_ERR;
    }
    if(!(reg_val & MPU6050_PWR_RESET_BIT))
    {
        //if reset was done
        reset_cnt=0;
        return MPU_OK;
    }
    return MPU_BUSY;
}

mpu_status_t mpu_init(void)
{
    // read who_am_i reg to see if we get the correct value ( sanity check )
    uint8_t who_am_i_val=0;
    if(i2c_read_bytes(MPU6050_I2C_ADDR, MPU6050_REG_WHO_AM_I, &who_am_i_val,1))
    {
        return MPU_I2C_ERR;
    }

    if(who_am_i_val != MPU6050_WHO_AM_I_VAL)
    {
        return MPU_INIT_ERR;
    }

    // config sensors
    if(i2c_write_reg(MPU6050_I2C_ADDR, MPU6050_REG_PWR_MGMT_1, MPU6050_CLK_PLL_X))
    {
        return MPU_I2C_ERR;
    }
    if(i2c_write_reg(MPU6050_I2C_ADDR, MPU6050_REG_CONFIG, MPU6050_DLPF_CFG_4))
    {
        return MPU_I2C_ERR;
    }
    if(i2c_write_reg(MPU6050_I2C_ADDR, MPU6050_REG_GYRO_CONFIG, MPU6050_SENSORS_VAL))
    {
        return MPU_I2C_ERR;
    }
    if(i2c_write_reg(MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_CONFIG, MPU6050_SENSORS_VAL))
    {
        return MPU_I2C_ERR;
    }   

    return MPU_OK;
}

mpu_status_t mpu_read_data(mpu_data_t *data)
{
    uint8_t buf[14];

    /* Step 1: Burst read all 14 sensor registers starting at ACCEL_XOUT_H (0x3B) */
    if (i2c_read_bytes(MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_XOUT_H, buf, 14) != 0) {
        return MPU_I2C_ERR;
    }

    /* Step 2: Unpack Big-Endian wire bytes into signed 16-bit local integers */
    int16_t raw_ax   = (int16_t)((buf[0]  << 8) | buf[1]);
    int16_t raw_ay   = (int16_t)((buf[2]  << 8) | buf[3]);
    int16_t raw_az   = (int16_t)((buf[4]  << 8) | buf[5]);
    int16_t raw_temp = (int16_t)((buf[6]  << 8) | buf[7]);
    int16_t raw_gx   = (int16_t)((buf[8]  << 8) | buf[9]);
    int16_t raw_gy   = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t raw_gz   = (int16_t)((buf[12] << 8) | buf[13]);

    /* Step 3: Convert raw ADC counts into physical floating-point units */
    data->ax = (float)raw_ax / MPU6050_ACCEL_SENS_2G;
    data->ay = (float)raw_ay / MPU6050_ACCEL_SENS_2G;
    data->az = (float)raw_az / MPU6050_ACCEL_SENS_2G;

    data->gx = (float)raw_gx / MPU6050_GYRO_SENS_250DPS;
    data->gy = (float)raw_gy / MPU6050_GYRO_SENS_250DPS;
    data->gz = (float)raw_gz / MPU6050_GYRO_SENS_250DPS;

    data->temp_c = ((float)raw_temp / 340.0f) + 36.53f;

    return MPU_OK;
}

mpu_status_t mpu_process(mpu_data_t * out_data)
{
    if(!out_data)
    {
        return MPU_INIT_ERR;
    }

    switch (MPU_State) {

        case MPU_STATE_UNINITIALISED:
            mpu_error_status = mpu_trigger_reset();
            if (mpu_error_status == MPU_OK) {
                MPU_State = MPU_STATE_RESET_WAIT;
                mpu_error_status = MPU_BUSY;
            } else {
                MPU_State = MPU_STATE_ERROR;
            }
            break;

        case MPU_STATE_RESET_WAIT: {
            mpu_error_status = mpu_check_reset();
            if (mpu_error_status == MPU_OK) {
                MPU_State = MPU_STATE_CONFIGURING;
            } else if (mpu_error_status != MPU_BUSY) {
                /* Hard failure (timeout or I2C NACK) */
                MPU_State = MPU_STATE_ERROR;
            }
            /* If MPU_BUSY, do nothing and stay in MPU_STATE_RESET_WAIT for next tick */
            break;
        }

        case MPU_STATE_CONFIGURING:
            mpu_error_status=mpu_init();
            if (mpu_error_status == MPU_OK) {
                MPU_State = MPU_STATE_READY;
            } else {
                MPU_State = MPU_STATE_ERROR;
            }
            break;

        case MPU_STATE_READY:
            mpu_error_status=mpu_read_data(out_data);
            if (mpu_error_status != MPU_OK) {
                // if transmission fails we do a reset
                MPU_State = MPU_STATE_UNINITIALISED;
            }
            break;

        case MPU_STATE_ERROR:
        default:
            // fault state
            MPU_State = MPU_STATE_UNINITIALISED;
            break;
    }

    return mpu_error_status;
}