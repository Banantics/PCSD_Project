#include "data_logger.h"

#include <stdio.h>
#include <string.h>

void DataLogger_PrintHeader(UART_HandleTypeDef* huart)
{
    if (!huart) return;

    const char* header = "time_ms,x_mg,y_mg,z_mg\r\n";
    HAL_UART_Transmit(huart, (uint8_t*)header, (uint16_t)strlen(header), 200);
}

void DataLogger_LogSample(UART_HandleTypeDef* huart,
                          const AccelerometerSampleMg* sample)
{
    if (!huart || !sample) return;

    char line[96];
    uint32_t timestamp = HAL_GetTick();

    int n = snprintf(line, sizeof(line),
                     "%lu,%d,%d,%d\r\n",
                     (unsigned long)timestamp,
                     (int)sample->x_mg,
                     (int)sample->y_mg,
                     (int)sample->z_mg);

    if (n <= 0) return;

    if (n >= (int)sizeof(line))
        n = (int)sizeof(line) - 1;

    HAL_UART_Transmit(huart, (uint8_t*)line, (uint16_t)n, 200);
}
