#include "serial_vcp.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

bool SerialVCP_Init(UART_HandleTypeDef* huart)
{
    if (!huart) return false;

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // USART1 on PB6(TX)/PB7(RX) -> ST-LINK VCP
    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    g.Mode = GPIO_MODE_AF_PP;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &g);

    huart->Instance = USART1;
    huart->Init.BaudRate = 115200;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    return (HAL_UART_Init(huart) == HAL_OK);
}

void SerialVCP_Write(UART_HandleTypeDef* huart, const char* text)
{
    if (!huart || !text) return;
    HAL_UART_Transmit(huart, (uint8_t*)text, (uint16_t)strlen(text), 200);
}

void SerialVCP_Printf(UART_HandleTypeDef* huart, const char* fmt, ...)
{
    if (!huart || !fmt) return;

    char buffer[160];

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (n <= 0) return;

    if (n >= (int)sizeof(buffer))
        n = (int)sizeof(buffer) - 1;

    HAL_UART_Transmit(huart, (uint8_t*)buffer, (uint16_t)n, 200);
}
