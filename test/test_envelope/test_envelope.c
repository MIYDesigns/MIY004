#include <unity.h>
#include "envelope.h"

void setUp(void) {}
void tearDown(void) {}

static uint16_t run(envelope_t *e, uint16_t target, int steps) {
    uint16_t v = 0;
    for (int i = 0; i < steps; i++) {
        v = envelope_step(e, target);
    }
    return v;
}

void test_attack_is_faster_than_decay(void) {
    envelope_t up, down;
    envelope_init(&up, 16384, 1024);  // 1/4 vs 1/64 per step
    envelope_init(&down, 16384, 1024);
    run(&down, 65535, 200);           // charge fully first

    uint16_t risen = run(&up, 65535, 10);
    uint16_t fallen = 65535 - run(&down, 0, 10);
    TEST_ASSERT_GREATER_THAN_UINT16(fallen, risen);
}

void test_reaches_target_and_holds(void) {
    envelope_t e;
    envelope_init(&e, 16384, 1024);
    TEST_ASSERT_EQUAL_UINT16(40000, run(&e, 40000, 500));
    TEST_ASSERT_EQUAL_UINT16(40000, run(&e, 40000, 10));
}

void test_decays_to_silence(void) {
    envelope_t e;
    envelope_init(&e, 65535, 2048);
    run(&e, 65535, 50);
    TEST_ASSERT_EQUAL_UINT16(0, run(&e, 0, 5000));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_attack_is_faster_than_decay);
    RUN_TEST(test_reaches_target_and_holds);
    RUN_TEST(test_decays_to_silence);
    return UNITY_END();
}
