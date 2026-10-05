#pragma once
// DAC1_OUT1 (PA4) -> TPA2005D1. Playback from flash is not written yet (DEV-82).
#include <stdbool.h>
#include <stdint.h>

void audio_init(void);
void audio_amp_enable(bool on);   // drives TPA2005D1 SD high/low
void audio_write(uint16_t level); // 12-bit, right-aligned
