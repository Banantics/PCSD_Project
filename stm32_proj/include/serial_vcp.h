#pragma once
#include "stm32l4xx_hal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Configures USART1 for the ST-LINK virtual COM port.
bool SerialVCP_Init(UART_HandleTypeDef* huart);

// Writes a null-terminated string over UART.
void SerialVCP_Write(UART_HandleTypeDef* huart, const char* text);

// Formats and writes text over UART.
void SerialVCP_Printf(UART_HandleTypeDef* huart, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
