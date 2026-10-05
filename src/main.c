// MIY004 product firmware - skeleton. Application logic lands under DEV-82.
#include "board.h"
#include "audio.h"
#include "hall.h"
#include "flash.h"
#include "envelope.h"

int main(void) {
    HAL_Init();
    miy_clock_init();
    miy_board_init();

    audio_init();
    flash_init();

    envelope_t env;
    envelope_init(&env, 16384, 512);

    for (;;) {
        // TODO(DEV-82): measure spin rate from U5, map to target, drive playback volume
        envelope_step(&env, 0);
        HAL_Delay(10);
    }
}
