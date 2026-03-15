#include "stm32l4xx_hal.h"
#include "board_support.h"
#include "serial_vcp.h"
#include "accelerometer_module.h"
#include "data_logger.h"

UART_HandleTypeDef huart1;

int main(void)
{
    Board_HAL_Init();
    Board_Led_Init();

    if (!SerialVCP_Init(&huart1))
        while (1) {}

    if (Accelerometer_Init() != ACCEL_OK)
    {
        while (1) {}
    }

    DataLogger_PrintHeader(&huart1);

    uint32_t next = HAL_GetTick();

    while (1)
    {
        next += 1;   // sample every 1 ms = ~1000 Hz

        AccelerometerSampleMg sample;

        if (Accelerometer_ReadMg(&sample) == ACCEL_OK)
        {
            DataLogger_LogSample(&huart1, &sample);
        }

        while ((int32_t)(HAL_GetTick() - next) < 0) {}

        Board_Led_Toggle();
    }
}
