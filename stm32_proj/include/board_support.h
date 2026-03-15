#pragma once
#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the HAL, system clock, and board LED.
void Board_HAL_Init(void);

// Configures the MCU system clock tree.
void Board_SystemClock_Config(void);

// Configures the on-board LED GPIO pin.
void Board_Led_Init(void);

// Drives the on-board LED to the ON state.
void Board_Led_On(void);

// Drives the on-board LED to the OFF state.
void Board_Led_Off(void);

// Toggles the current on-board LED state.
void Board_Led_Toggle(void);

#ifdef __cplusplus
}
#endif
