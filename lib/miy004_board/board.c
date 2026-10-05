#include "board.h"
#include "pins.h"

static void clock_fault(void) {
    __disable_irq();
    for (;;) {
    }
}

void miy_clock_init(void) {
    __HAL_RCC_PWR_CLK_ENABLE();
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        clock_fault();
    }

    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_MSI | RCC_OSCILLATORTYPE_HSI48;
    osc.MSIState = RCC_MSI_ON;
    osc.MSIClockRange = RCC_MSIRANGE_11;  // 48 MHz; USB needs HCLK >= 14.2 MHz
    osc.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    osc.HSI48State = RCC_HSI48_ON;
    osc.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        clock_fault();
    }

    RCC_ClkInitTypeDef clk = {0};
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) {
        clock_fault();
    }

    RCC_PeriphCLKInitTypeDef per = {0};
    per.PeriphClockSelection = RCC_PERIPHCLK_USB;
    per.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    if (HAL_RCCEx_PeriphCLKConfig(&per) != HAL_OK) {
        clock_fault();
    }

    // CRS: trim HSI48 against the 1 kHz USB start-of-frame
    __HAL_RCC_CRS_CLK_ENABLE();
    RCC_CRSInitTypeDef crs = {0};
    crs.Prescaler = RCC_CRS_SYNC_DIV1;
    crs.Source = RCC_CRS_SYNC_SOURCE_USB;
    crs.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
    crs.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000, 1000);
    crs.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
    crs.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;
    HAL_RCCEx_CRSConfig(&crs);
}

void miy_board_init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    // Outputs default low: amp shut down, hall latch unpowered, flash rail off
    HAL_GPIO_WritePin(AMP_EN_PORT, AMP_EN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(V_HALL_PORT, V_HALL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STOR_IN_PORT, STOR_IN_PIN, GPIO_PIN_RESET);

    GPIO_InitTypeDef g = {0};
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    g.Pin = AMP_EN_PIN | STOR_IN_PIN;
    HAL_GPIO_Init(GPIOB, &g);
    g.Pin = V_HALL_PIN;
    HAL_GPIO_Init(V_HALL_PORT, &g);

    g.Mode = GPIO_MODE_INPUT;
    g.Pin = SIG_HALL_PIN | WAKE_HALL_PIN;
    HAL_GPIO_Init(GPIOA, &g);
    g.Pin = SIG_MUX_PIN;
    HAL_GPIO_Init(SIG_MUX_PORT, &g);
}

bool miy_hsi48_ready(void) {
    return __HAL_RCC_GET_FLAG(RCC_FLAG_HSI48RDY) != 0;
}
