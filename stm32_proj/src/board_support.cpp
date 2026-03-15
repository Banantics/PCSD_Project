#include "board_support.h"

// PA5 = Arduino D13 / LED1 on B-L475E-IOT01A
static constexpr uint16_t LED_PIN = GPIO_PIN_5;
static GPIO_TypeDef* LED_PORT = GPIOA;

void Board_HAL_Init(void)
{
    HAL_Init();
    Board_SystemClock_Config();
    Board_Led_Init();
}

// Keep SysTick handler here so main stays clean.
extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

void Board_Led_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Pin = LED_PIN;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &g);

    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

void Board_Led_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

void Board_Led_On(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void Board_Led_Off(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}


void Board_SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    osc.MSIState = RCC_MSI_ON;
    osc.MSIClockRange = RCC_MSIRANGE_6; // 4 MHz source for PLL
    osc.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;

    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    osc.PLL.PLLM = 1;
    osc.PLL.PLLN = 40;
    osc.PLL.PLLP = RCC_PLLP_DIV7;
    osc.PLL.PLLQ = RCC_PLLQ_DIV2;
    osc.PLL.PLLR = RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        while (1) {}

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_4) != HAL_OK)
        while (1) {}

    HAL_RCCEx_EnableMSIPLLMode();
}
