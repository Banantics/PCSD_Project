#include "accelerometer_module.h"

#include <stdio.h>
#include <string.h>

extern "C" {
#include "stm32l475e_iot01_accelero.h"
}

static bool g_accel_ready = false;

static void uart_write(UART_HandleTypeDef *huart, const char *text)
{
    if (huart == nullptr || text == nullptr)
        return;

    HAL_UART_Transmit(huart, (uint8_t *)text, (uint16_t)strlen(text), 200);
}

bool AccelerometerModule_Init(void)
{
    g_accel_ready = (BSP_ACCELERO_Init() == ACCELERO_OK);
    return g_accel_ready;
}

bool AccelerometerModule_Read(AccelerometerSampleMg *sample)
{
    if (!g_accel_ready || sample == nullptr)
        return false;

    int16_t xyz[3] = {0, 0, 0};
    BSP_ACCELERO_AccGetXYZ(xyz);

    sample->x_mg = xyz[0];
    sample->y_mg = xyz[1];
    sample->z_mg = xyz[2];
    return true;
}

void AccelerometerModule_Print(UART_HandleTypeDef *huart, const AccelerometerSampleMg *sample)
{
    if (sample == nullptr)
        return;

    char line[96];
    int n = snprintf(
        line,
        sizeof(line),
        "ACC mg: X=%d Y=%d Z=%d\r\n",
        (int)sample->x_mg,
        (int)sample->y_mg,
        (int)sample->z_mg);

    if (n <= 0)
        return;

    uart_write(huart, line);
}

void AccelerometerModule_ReadAndPrint(UART_HandleTypeDef *huart)
{
    AccelerometerSampleMg sample;
    if (!AccelerometerModule_Read(&sample))
    {
        uart_write(huart, "ACC read failed\r\n");
        return;
    }

    AccelerometerModule_Print(huart, &sample);
}
