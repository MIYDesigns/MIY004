#include "stm32l4xx_hal.h"

// HAL timebase. library.json sets libArchive=false: in an archive nothing references
// this object, so the linker would keep the startup file's weak default instead and
// HAL_Delay() would hang.
void SysTick_Handler(void) {
    HAL_IncTick();
}

void HardFault_Handler(void) {
    for (;;) {
    }
}
