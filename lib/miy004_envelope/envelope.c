#include "envelope.h"

void envelope_init(envelope_t *e, uint16_t attack, uint16_t decay) {
    e->attack = attack;
    e->decay = decay;
    e->level = 0;
}

uint16_t envelope_step(envelope_t *e, uint16_t target) {
    uint32_t t = (uint32_t)target << 16;
    if (t > e->level) {
        uint64_t step = ((uint64_t)(t - e->level) * e->attack) >> 16;
        e->level += step ? (uint32_t)step : 1;
    } else if (t < e->level) {
        uint64_t step = ((uint64_t)(e->level - t) * e->decay) >> 16;
        e->level -= step ? (uint32_t)step : 1;
    }
    return (uint16_t)(e->level >> 16);
}
