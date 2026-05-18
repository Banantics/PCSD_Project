#include "i2c_ttgo.h"

#include "stm32l4xx_hal.h"

static I2C_HandleTypeDef hi2c1 = {};

bool I2CTTGO::Init()
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {};
    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;          // PB8=SCL, PB9=SDA
    gpio.Mode = GPIO_MODE_AF_OD;                 // I2C must be open-drain
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;

    HAL_GPIO_Init(GPIOB, &gpio);

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00303D5B;              // 100 kHz-ish for 4 MHz clock
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        return false;
    }

    HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE);
    HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0);
    last_error_ = HAL_I2C_ERROR_NONE;

    return true;
}

bool I2CTTGO::SendState(uint8_t state)
{
    const HAL_StatusTypeDef result =
        HAL_I2C_Master_Transmit(&hi2c1, TTGO_ADDR, &state, 1, 100);

    if (result == HAL_OK)
    {
        last_error_ = HAL_I2C_ERROR_NONE;
        return true;
    }

    last_error_ = HAL_I2C_GetError(&hi2c1);
    return false;
}

uint32_t I2CTTGO::GetLastError() const
{
    return last_error_;
}
