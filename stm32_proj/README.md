# ST Repos Used By This Project

This project uses ST code that comes from the `framework-stm32cubel4` package in PlatformIO.

The important upstream ST repos are:

- `STMicroelectronics/b-l475e-iot01a-bsp`
- `STMicroelectronics/stm32-lsm6dsl`
- `STMicroelectronics/stm32l4xx-hal-driver`

The current project uses all three layers:

1. BSP functions for the accelerometer
2. LSM6DSL component-driver functions underneath the BSP
3. HAL functions directly in this project for timer, clock, GPIO, UART, and tick handling

## 1. Board BSP Repo

Repo:

- <https://github.com/STMicroelectronics/b-l475e-iot01a-bsp>

Main files:

- <https://github.com/STMicroelectronics/b-l475e-iot01a-bsp/blob/main/stm32l475e_iot01_accelero.h>
- <https://github.com/STMicroelectronics/b-l475e-iot01a-bsp/blob/main/stm32l475e_iot01_accelero.c>
- <https://github.com/STMicroelectronics/b-l475e-iot01a-bsp/blob/main/stm32l475e_iot01.c>

### Public BSP accelerometer functions used by this project

```c
ACCELERO_StatusTypeDef BSP_ACCELERO_Init(void);
```

- What it does: checks that the accelerometer is present, selects the LSM6DSL driver, and configures the board-level accelerometer interface.
- Used in this project from: `Accelerometer::Init()`

```c
void BSP_ACCELERO_AccGetXYZ(int16_t *pDataXYZ);
```

- What it does: returns X, Y, and Z acceleration values through the pointer argument.
- Used in this project from: `Accelerometer::Read()`

### Internal board-level helper functions from the BSP repo that are also used underneath

These are not called directly by your application, but they are part of the actual call chain.

```c
void SENSOR_IO_Init(void);
```

- What it does: initializes the shared sensor I2C bus on the board.

```c
void SENSOR_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
```

- What it does: writes one register value to a sensor over I2C.

```c
uint8_t SENSOR_IO_Read(uint8_t Addr, uint8_t Reg);
```

- What it does: reads one register value from a sensor over I2C.

```c
uint16_t SENSOR_IO_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length);
```

- What it does: reads several sensor registers in one I2C transaction.

### How this BSP layer is used in your code

Your code calls:

- `BSP_ACCELERO_Init()`
- `BSP_ACCELERO_AccGetXYZ()`

Those functions then call the lower sensor driver in the next repo.

## 2. LSM6DSL Component Driver Repo

Repo:

- <https://github.com/STMicroelectronics/stm32-lsm6dsl>

Main files:

- <https://github.com/STMicroelectronics/stm32-lsm6dsl/blob/main/lsm6dsl.h>
- <https://github.com/STMicroelectronics/stm32-lsm6dsl/blob/main/lsm6dsl.c>

### Component-driver functions used by this project

These are used indirectly through the BSP layer.

```c
void LSM6DSL_AccInit(uint16_t InitStruct);
```

- What it does: configures the LSM6DSL accelerometer registers, mainly output data rate and full-scale.

```c
uint8_t LSM6DSL_AccReadID(void);
```

- What it does: reads the sensor `WHO_AM_I` register so the BSP can confirm the device is really an LSM6DSL.

```c
void LSM6DSL_AccReadXYZ(int16_t *pData);
```

- What it does: reads the raw X, Y, and Z accelerometer output registers and converts them into acceleration values.

### How this component layer is used in your code

Your code does not call these functions directly.

The path is:

1. `BSP_ACCELERO_Init()` -> `LSM6DSL_AccReadID()` and `LSM6DSL_AccInit()`
2. `BSP_ACCELERO_AccGetXYZ()` -> `LSM6DSL_AccReadXYZ()`

## 3. STM32L4 HAL Driver Repo

Repo:

- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver>

Main headers for the functions used here:

- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver/blob/main/Inc/stm32l4xx_hal.h>
- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver/blob/main/Inc/stm32l4xx_hal_tim.h>
- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver/blob/main/Inc/stm32l4xx_hal_rcc.h>
- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver/blob/main/Inc/stm32l4xx_hal_rcc_ex.h>
- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver/blob/main/Inc/stm32l4xx_hal_gpio.h>
- <https://github.com/STMicroelectronics/stm32l4xx-hal-driver/blob/main/Inc/stm32l4xx_hal_uart.h>

### HAL functions called directly by this project

These are called directly from your own source files, not through the BSP.

```c
HAL_StatusTypeDef HAL_Init(void);
```

- What it does: starts the HAL library and basic MCU support.

```c
HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
```

- What it does: configures the base timer peripheral used for sampling.

```c
HAL_StatusTypeDef HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);
```

- What it does: starts the timer in interrupt mode.

```c
void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim);
```

- What it does: handles the timer interrupt and dispatches HAL timer callbacks.

```c
void HAL_IncTick(void);
```

- What it does: increments the HAL millisecond tick counter.

```c
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
```

- What it does: configures the oscillator and PLL settings.

```c
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);
```

- What it does: configures the CPU and bus clocks.

```c
void HAL_RCCEx_EnableMSIPLLMode(void);
```

- What it does: enables MSI PLL mode after clock setup.

```c
uint32_t HAL_GetTick(void);
```

- What it does: returns the current HAL tick value in milliseconds.

```c
void HAL_Delay(uint32_t Delay);
```

- What it does: blocks for the requested number of milliseconds.

```c
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
```

- What it does: configures GPIO pins, used here for the UART pins.

```c
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);
```

- What it does: configures the UART peripheral.

```c
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

- What it does: sends bytes over UART.

### HAL functions used indirectly through the BSP sensor path

These are not called directly by your own `.cpp` files, but they are used underneath the accelerometer read path.

```c
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
```

- What it does: initializes the I2C peripheral used to talk to the on-board sensor.

```c
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c);
```

- What it does: deinitializes the I2C peripheral.

```c
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

- What it does: writes one or more bytes to a device register over I2C.

```c
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

- What it does: reads one or more bytes from a device register over I2C.

```c
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
```

- What it does: checks whether the I2C device acknowledges on the bus.

```c
HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter);
```

- What it does: configures the I2C analog filter during bus setup.

## 4. Exact Call Chain Used By This Project

Accelerometer path:

1. `Accelerometer::Init()`
2. `BSP_ACCELERO_Init()`
3. `LSM6DSL_AccReadID()`
4. `LSM6DSL_AccInit()`

Accelerometer sample read path:

1. `Accelerometer::Read()`
2. `BSP_ACCELERO_AccGetXYZ()`
3. `LSM6DSL_AccReadXYZ()`
4. `SENSOR_IO_Read()` and `SENSOR_IO_ReadMultiple()`
5. `HAL_I2C_Mem_Read()` and related HAL I2C functions inside the BSP

Timer and serial path used directly by your code:

1. `HAL_Init()`
2. `HAL_RCC_OscConfig()`
3. `HAL_RCC_ClockConfig()`
4. `HAL_TIM_Base_Init()`
5. `HAL_TIM_Base_Start_IT()`
6. `HAL_TIM_IRQHandler()`
7. `HAL_IncTick()`
8. `HAL_GetTick()`
9. `HAL_GPIO_Init()`
10. `HAL_UART_Init()`
11. `HAL_UART_Transmit()`

## 5. Important Clarification

`BSP_ACCELERO_AccGetXYZ()` is not a datasheet function.

It is an ST software API declared in:

- <https://github.com/STMicroelectronics/b-l475e-iot01a-bsp/blob/main/stm32l475e_iot01_accelero.h>

It is implemented in:

- <https://github.com/STMicroelectronics/b-l475e-iot01a-bsp/blob/main/stm32l475e_iot01_accelero.c>

The actual sensor register work happens lower down in:

- <https://github.com/STMicroelectronics/stm32-lsm6dsl/blob/main/lsm6dsl.c>

So the correct way to look it up is:

1. find the public declaration in the `.h` file
2. open the matching `.c` file for the implementation
3. follow the call into the lower-level driver if you want the real hardware access details
