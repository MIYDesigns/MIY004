#pragma once
#include <stdbool.h>
#include "stm32l4xx_hal.h"

// 48 MHz SYSCLK from MSI; USB clocked from HSI48 trimmed by the CRS.
// There is no crystal on MIY004: without the CRS locked to USB SOF,
// enumeration fails silently on good hardware.
void miy_clock_init(void);

// GPIO clocks and safe default pin states (amp off, hall unpowered).
void miy_board_init(void);

bool miy_hsi48_ready(void);
