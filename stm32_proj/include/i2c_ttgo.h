#pragma once

#include <cstdint>

class I2CTTGO
{
public:
    bool Init();
    bool SendState(uint8_t state);
    uint32_t GetLastError() const;

private:
    static constexpr uint16_t TTGO_ADDR = 0x55 << 1; // STM32 HAL wants 8-bit shifted address
    uint32_t last_error_ = 0;
};
