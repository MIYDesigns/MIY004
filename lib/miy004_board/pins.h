#pragma once
// MIY004 pin map. Source of truth: MAIN-PCB wiring guide (knowledge MIY004 ID 31).
// Change it there first, then here.

#include "stm32l4xx_hal.h"

// Hall sensing
#define SIG_HALL_PORT    GPIOA        // U5 DRV5011 latch, rate - EXTI0 / WKUP1
#define SIG_HALL_PIN     GPIO_PIN_0
#define V_HALL_PORT      GPIOA        // U5 supply, GPIO-powered (duty-cycled)
#define V_HALL_PIN       GPIO_PIN_1
#define WAKE_HALL_PORT   GPIOA        // U3 DRV5032FB wake switch - EXTI2 / WKUP4
#define WAKE_HALL_PIN    GPIO_PIN_2

// Audio
#define AMP_IN_PORT      GPIOA        // DAC1_OUT1 -> TPA2005D1 input
#define AMP_IN_PIN       GPIO_PIN_4
#define AMP_EN_PORT      GPIOB        // TPA2005D1 SD - amp is default-off (pulldown)
#define AMP_EN_PIN       GPIO_PIN_0

// Power
#define SIG_MUX_PORT     GPIOC        // mux ST - EXTI13 / WKUP2; rising = USB connected
#define SIG_MUX_PIN      GPIO_PIN_13
#define STOR_IN_PORT     GPIOB        // switched flash rail (VFLASH) enable
#define STOR_IN_PIN      GPIO_PIN_1

// Flash - SPI2 (AF5)
#define FLASH_CS_PORT    GPIOB
#define FLASH_CS_PIN     GPIO_PIN_12
#define FLASH_SPI_PORT   GPIOB
#define FLASH_SCK_PIN    GPIO_PIN_13
#define FLASH_MISO_PIN   GPIO_PIN_14
#define FLASH_MOSI_PIN   GPIO_PIN_15

// Debug / boot - fixed by silicon or ROM bootloader, do not remap
#define UART_PORT        GPIOA        // USART1 (AF7)
#define UART_TX_PIN      GPIO_PIN_9
#define UART_RX_PIN      GPIO_PIN_10
#define USB_DM_PIN       GPIO_PIN_11  // GPIOA
#define USB_DP_PIN       GPIO_PIN_12  // GPIOA
#define SWDIO_PIN        GPIO_PIN_13  // GPIOA
#define SWCLK_PIN        GPIO_PIN_14  // GPIOA
#define SWO_PIN          GPIO_PIN_3   // GPIOB
// BOOT0 is PH3. PB2 is free (vacated by SIG-MUX).
