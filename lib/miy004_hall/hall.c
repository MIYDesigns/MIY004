#include "hall.h"
#include "pins.h"
#include "stm32l4xx_hal.h"

void hall_power(bool on) {
    HAL_GPIO_WritePin(V_HALL_PORT, V_HALL_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool hall_rate_level(void) {
    return HAL_GPIO_ReadPin(SIG_HALL_PORT, SIG_HALL_PIN) == GPIO_PIN_SET;
}

bool hall_wake_level(void) {
    return HAL_GPIO_ReadPin(WAKE_HALL_PORT, WAKE_HALL_PIN) == GPIO_PIN_SET;
}
