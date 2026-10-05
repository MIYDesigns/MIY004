#pragma once
// Spin rate -> volume, fast attack / slow decay. Pure C, no hardware: unit-tested on host.
#include <stdint.h>

typedef struct {
    uint16_t attack;  // Q16 fraction of the gap closed per step when rising
    uint16_t decay;   // Q16 fraction of the gap closed per step when falling
    uint32_t level;   // Q16, 0..65535 << 16 scale held internally
} envelope_t;

void envelope_init(envelope_t *e, uint16_t attack, uint16_t decay);

// Advance one step toward target (0..65535). Returns the new level (0..65535).
uint16_t envelope_step(envelope_t *e, uint16_t target);
