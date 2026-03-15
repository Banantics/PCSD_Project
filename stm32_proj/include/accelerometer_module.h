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

typedef enum
{
    ACCEL_OK = 0,
    ACCEL_ERR_NOT_INIT,
    ACCEL_ERR_INIT_FAILED,
    ACCEL_ERR_READ_FAILED
} AccelerometerStatus;

// Initializes the accelerometer hardware and driver state.
AccelerometerStatus Accelerometer_Init(void);

// Returns true when the accelerometer is ready to be read.
bool Accelerometer_IsReady(void);

// Reads one accelerometer sample in milligravity units.
AccelerometerStatus Accelerometer_ReadMg(AccelerometerSampleMg *sample);

// Converts a status code into a readable string.
const char* Accelerometer_StatusString(AccelerometerStatus status);

#ifdef __cplusplus
}
#endif
