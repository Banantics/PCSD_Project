# STM32 Accelerometer Data Logger

This project runs on the `B-L475E-IOT01A` discovery board using PlatformIO and the STM32Cube framework. It reads acceleration data from the on-board LSM6DSL sensor, formats each sample as a CSV row, and sends the data over the ST-LINK virtual COM port at `115200` baud.

The serial output is intended to be captured on the PC side. With the current `platformio.ini`, PlatformIO's `log2file` monitor filter can save monitor output into log files during a serial monitor session.

## How the project works

1. `main.cpp` starts the HAL, system clock, LED, UART, and accelerometer.
2. The program prints a CSV header: `time_ms,x_mg,y_mg,z_mg`.
3. The main loop runs roughly every `1 ms` for an effective sample rate near `1000 Hz`.
4. Each loop reads one accelerometer sample in milligravity units.
5. The sample is sent over UART as a CSV line with the current tick count.
6. The board LED toggles every loop to show that the firmware is still running.

## File overview

### Top level

- `platformio.ini`: PlatformIO environment setup for the STM32 board, BSP include paths, ST-LINK upload/debug settings, and serial monitor configuration.
- `README.md`: Project overview, file descriptions, and runtime behaviour.
- `.gitignore`: Ignores build/editor output and log file contents.

### include

- `include/accelerometer_module.h`: Public API for initializing the accelerometer, checking readiness, reading XYZ samples, and converting status codes to text.
- `include/board_support.h`: Public API for HAL startup, system clock configuration, and on-board LED control.
- `include/data_logger.h`: Public API for printing the CSV header and logging one formatted accelerometer sample.
- `include/serial_vcp.h`: Public API for configuring and writing to the USART1 ST-LINK virtual COM port.

### src

- `src/main.cpp`: Application entry point and the main 1 kHz acquisition loop.
- `src/accelerometer_module.cpp`: Wraps the STM32Cube BSP accelerometer driver and exposes a small project-specific interface.
- `src/board_support.cpp`: Initializes the HAL, configures the system clock, defines the SysTick handler, and manages LED GPIO control.
- `src/data_logger.cpp`: Formats timestamps and accelerometer readings into CSV and transmits them over UART.
- `src/serial_vcp.cpp`: Configures USART1 on `PB6/PB7` and provides simple UART string/printf helpers.
- `src/stm32cube_bsp_accelero.c`: Pulls the STM32Cube accelerometer BSP implementation into the build.
- `src/stm32cube_bsp_board.c`: Pulls the STM32Cube board support implementation into the build.
- `src/stm32cube_bsp_lsm6dsl.c`: Pulls the STM32Cube LSM6DSL sensor driver into the build.

### logs

- `logs/.gitkeep`: Placeholder so the `logs` directory can exist in the repository while generated log files stay ignored.
- `logs/*.log`: Serial monitor output logs generated during PC-side capture. These are intentionally not committed.

## Data format

The firmware sends CSV lines in this form:

```text
time_ms,x_mg,y_mg,z_mg
1234,-15,7,1002
```

- `time_ms`: HAL tick count in milliseconds since startup.
- `x_mg`, `y_mg`, `z_mg`: Accelerometer readings in milligravity units.
