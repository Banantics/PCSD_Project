#include <cstdio>
#include <stdint.h>

#include "board_support.h"
#include "i2c_ttgo.h"
#include "serial_vcp.h"

static void ReportError(SerialVCP& serial, const char* message);
static void ReportI2CError(SerialVCP& serial, uint32_t error);
static void ErrorLoop(SerialVCP& serial, const char* message);

int main(void)
{
    Board_Setup();

    SerialVCP pc_serial;
    I2CTTGO ttgo_i2c;

    if (!pc_serial.Init())
    {
        ErrorLoop(pc_serial, "ST-LINK VCP init failed");
    }

    pc_serial.Write("\r\nSTM32 booted\r\n");

    if (!ttgo_i2c.Init())
    {
        ErrorLoop(pc_serial, "TTGO I2C init failed");
    }

    pc_serial.Write("TTGO I2C test started\r\n");
    pc_serial.Write("Sending state 1 once per second\r\n");

            uint8_t value = 1;


    while (1)
    {

        value++;


        if(value==4) value=0;
        if (ttgo_i2c.SendState(value))
        {
            pc_serial.Write("TTGO send OK\r\n");
        }
        else
        {
            ReportI2CError(pc_serial, ttgo_i2c.GetLastError());
        }

        HAL_Delay(1000);
    }
}

static void ReportI2CError(SerialVCP& serial, uint32_t error)
{
    char message[64] = {};
    std::snprintf(message,
                  sizeof(message),
                  "ERROR,TTGO I2C send failed,0x%08lX\r\n",
                  static_cast<unsigned long>(error));
    serial.Write(message);

    if ((error & HAL_I2C_ERROR_AF) != 0U)
    {
        serial.Write("ERROR,No ACK from TTGO; check address, wiring, and TTGO SDA/SCL pins\r\n");
    }
}

static void ReportError(SerialVCP& serial, const char* message)
{
    if (!serial.IsReady())
    {
        return;
    }

    serial.Write("ERROR,");
    serial.Write(message);
    serial.Write("\r\n");
}

static void ErrorLoop(SerialVCP& serial, const char* message)
{
    ReportError(serial, message);

    while (1)
    {
        HAL_Delay(100);
    }
}
