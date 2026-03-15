#include "accelerometer_module.h"

extern "C" {
#include "stm32l475e_iot01_accelero.h"
}

static bool g_ready = false;

AccelerometerStatus Accelerometer_Init(void)
{
    if (BSP_ACCELERO_Init() != ACCELERO_OK)
    {
        g_ready = false;
        return ACCEL_ERR_INIT_FAILED;
    }

    g_ready = true;
    return ACCEL_OK;
}

bool Accelerometer_IsReady(void)
{
    return g_ready;
}

AccelerometerStatus Accelerometer_ReadMg(AccelerometerSampleMg *sample)
{
    if (!g_ready) return ACCEL_ERR_NOT_INIT;
    if (!sample)  return ACCEL_ERR_READ_FAILED;

    int16_t xyz[3] = {0, 0, 0};

    // BSP call (already handles the sensor + bus)
    BSP_ACCELERO_AccGetXYZ(xyz);

    sample->x_mg = xyz[0];
    sample->y_mg = xyz[1];
    sample->z_mg = xyz[2];

    return ACCEL_OK;
}

const char* Accelerometer_StatusString(AccelerometerStatus s)
{
    switch (s)
    {
        case ACCEL_OK:                 return "ACCEL_OK";
        case ACCEL_ERR_NOT_INIT:       return "ACCEL_ERR_NOT_INIT";
        case ACCEL_ERR_INIT_FAILED:    return "ACCEL_ERR_INIT_FAILED";
        case ACCEL_ERR_READ_FAILED:    return "ACCEL_ERR_READ_FAILED";
        default:                       return "ACCEL_ERR_UNKNOWN";
    }
}
