#pragma once

#include "stm32l4xx_hal.h"
#include "accelerometer_module.h"

#ifdef __cplusplus
extern "C" {
#endif

// Sends the CSV header row over UART.
void DataLogger_PrintHeader(UART_HandleTypeDef* huart);

// Sends one accelerometer sample as a CSV line.
void DataLogger_LogSample(UART_HandleTypeDef* huart,
                          const AccelerometerSampleMg* sample);

#ifdef __cplusplus
}
#endif
