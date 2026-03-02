#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int16_t x_mg;
    int16_t y_mg;
    int16_t z_mg;
} AccelerometerSampleMg;

bool AccelerometerModule_Init(void);
bool AccelerometerModule_Read(AccelerometerSampleMg *sample);
void AccelerometerModule_Print(UART_HandleTypeDef *huart, const AccelerometerSampleMg *sample);
void AccelerometerModule_ReadAndPrint(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif
