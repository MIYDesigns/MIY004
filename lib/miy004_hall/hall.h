#pragma once
// U5 DRV5011 latch (rate, GPIO-powered on V_HALL) and U3 DRV5032FB (wake).
// Rate measurement and duty-cycling are not written yet: the duty cycle depends on
// the DEV-70 edge-count measurement (see MIY004-DEV bench/hall_window_replay.py).
#include <stdbool.h>

void hall_power(bool on);   // U5 supply via V_HALL
bool hall_rate_level(void); // U5 output
bool hall_wake_level(void); // U3 output (active low)
