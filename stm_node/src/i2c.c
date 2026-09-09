#include "i2c.h"

void I2C1_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN_Msk;
    GPIOB->CRL |= GPIO_CRL_MODE6_Msk | GPIO_CRL_MODE7_Msk;
    GPIOB->CRL |= GPIO_CRL_CNF6_Msk | GPIO_CRL_CNF7_Msk;
    //configure pins 6 and 7 to Alternate function open drain, max output speed

    I2C1->CR1 &= ~I2C_CR1_PE_Msk;
    // disable peripheric, to modify it

    I2C1->CR2 |= 0x24;
    //set out own clock value in the config reg
    I2C1->CCR |= 0xB4;
    // configure 100kHz peripheral clock, in Standard mode
    I2C1->TRISE |= 0x25;
    // max SCL rise time; this is our APB1 clock val + 1

    I2C1->CR1 |= I2C_CR1_PE_Msk;
}

int8_t i2c_write_reg(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
    uint32_t timeout = 10000;
    while(I2C1->SR2 & I2C_SR2_BUSY_Msk)
    {
        // if busy ( communication already ongoing ), give it some time
        // if still busy, return error

        if(--timeout == 0)
        {
            return I2C_ERR_BUSY;
        }
    }

    I2C1->CR1 |= I2C_CR1_START_Msk;

    while(!(I2C1->SR1 & I2C_SR1_SB_Msk));
    // SB(start bit)=1, means SDA line is pulled low by hardware and the start bit was successfully sent

    I2C1->DR = slave_addr << 1;
    // address is on 7 bits, so for the LSB we put a 0 cuz we are in write mode
    // 0 for write mode, 1 for read mode ( for LSB )

    while(!(I2C1->SR1 & I2C_SR1_ADDR_Msk))
    {
        //check if address matched, we check AF bit ( Acknowledge failed ) 
        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            // clear flag and send STOP
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_ADDR;
        }
    }

    (void)I2C1->SR1;
    (void)I2C1->SR2;
    // read SR1 and SR2 to clear ADDR

    while(!(I2C1->SR1 & I2C_SR1_TXE_Msk))
    {
        // wait until the DR is empty and we can send data again
        // or if we don't get an ACK there is an error and exit early

        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            // clear flag and send STOP
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_DATA;
        }
    }


    // now the shifting was done and DR can be written to again, we write the reg addr
    I2C1->DR = reg_addr;

    while(!(I2C1->SR1 & I2C_SR1_TXE_Msk))
    {

        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_DATA;
        }
    }

    I2C1->DR = data;

    //now we have to wait until we see BTF ( so data was sent and there is no more data to fetch cuz we are done)

    while(!(I2C1->SR1 & I2C_SR1_BTF_Msk))
    {

        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_DATA;
        }
    }

    I2C1->CR1 |= I2C_CR1_STOP_Msk;
    // send stop bit, this also clears BTF

    return I2C_OK;
}

int8_t i2c_read_bytes(uint8_t slave_addr, uint8_t start_reg, uint8_t *data, uint16_t len)
{
    // as in write function, we send both the sensor's and then register's address
    // this time we put 1 as LSB cuz we are in Read Mode

    if(len<=0) return I2C_BAD_INPUT;

    uint32_t timeout = 10000;
    while(I2C1->SR2 & I2C_SR2_BUSY_Msk)
    {
        if(--timeout == 0)
        {
            return I2C_ERR_BUSY;
        }
    }

    I2C1->CR1 |= I2C_CR1_START_Msk;

    while(!(I2C1->SR1 & I2C_SR1_SB_Msk));

    I2C1->DR = slave_addr << 1 | 0; // 0 cuz we are writing firstly the start reg addr

    while(!(I2C1->SR1 & I2C_SR1_ADDR_Msk))
    {
        //check if address matched, we check AF bit ( Acknowledge failed ) 
        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            // clear flag and send STOP
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_ADDR;
        }
    }

    (void)I2C1->SR1;
    (void)I2C1->SR2;

    while(!(I2C1->SR1 & I2C_SR1_TXE_Msk))
    {
        // wait until the DR is empty and we can send data again
        // or if we don't get an ACK there is an error and exit early

        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            // clear flag and send STOP
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_DATA;
        }
    }

    // now the shifting was done and DR can be written to again, we write the reg addr
    I2C1->DR = start_reg;

    while(!(I2C1->SR1 & I2C_SR1_TXE_Msk))
    {

        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_DATA;
        }
    }


    // send START again ( repeated START ) 
    I2C1->CR1 |= I2C_CR1_START_Msk;
    while (!(I2C1->SR1 & I2C_SR1_SB_Msk));

    //send the slave addr but with a 1 for Read Mode
    I2C1->DR = slave_addr << 1 | 1;

    while(!(I2C1->SR1 & I2C_SR1_ADDR_Msk))
    {
        //check if address matched, we check AF bit ( Acknowledge failed ) 
        if(I2C1->SR1 & I2C_SR1_AF_Msk)
        {
            // clear flag and send STOP
            I2C1->SR1 &= ~I2C_SR1_AF_Msk;
            I2C1->CR1 |= I2C_CR1_STOP_Msk;
            return I2C_ERR_NACK_ADDR;
        }
    }


    if(len == 1)
    {
        // reading one byte is a special case since we have to send NACK on the 2nd to last byte, and sthen STOP
        // so in this case we have to send it before reading anything
        I2C1->CR1 &= ~I2C_CR1_ACK_Msk;

        //clear ADDR
        (void)I2C1->SR1;
        (void)I2C1->SR2;

        I2C1->CR1 |= I2C_CR1_STOP_Msk;

        //read the only byte
        while(!(I2C1->SR1 & I2C_SR1_RXNE_Msk));
        data[0] = I2C1->DR;
    }
    else
    {

        I2C1->CR1 |= I2C_CR1_ACK_Msk; //enavle ACK generation ( to send ACK automatically after each byte read )
        //clear ADDR
        (void)I2C1->SR1;
        (void)I2C1->SR2;

        for (int i=0; i<len - 1; ++i)
        {
            while(!(I2C1->SR1 & I2C_SR1_RXNE_Msk));
            // poll RxNE until DR is filled with data we want to read from
            // this keeps reading one byte of each subsequent register

            data[i]=I2C1->DR;

            //ACK automatically sent after every byte read
        }

        // before reading the last byte, send NACK and then STOP
        I2C1->CR1 &= ~I2C_CR1_ACK_Msk;
        I2C1->CR1 |= I2C_CR1_STOP_Msk;
        while(!(I2C1->SR1 & I2C_SR1_RXNE_Msk));
        data[len-1]=I2C1->DR;

    }

    return I2C_OK;
}