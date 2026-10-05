#include "audio.h"
#include "pins.h"
#include "stm32l4xx_hal.h"

static DAC_HandleTypeDef dac;

void audio_init(void) {
    __HAL_RCC_DAC1_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Pin = AMP_IN_PIN;
    g.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(AMP_IN_PORT, &g);

    dac.Instance = DAC1;
    HAL_DAC_Init(&dac);

    DAC_ChannelConfTypeDef ch = {0};
    ch.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    ch.DAC_Trigger = DAC_TRIGGER_NONE;
    ch.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    ch.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    ch.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    HAL_DAC_ConfigChannel(&dac, &ch, DAC_CHANNEL_1);

    HAL_DAC_SetValue(&dac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);  // mid-rail, no pop
    HAL_DAC_Start(&dac, DAC_CHANNEL_1);
}

void audio_amp_enable(bool on) {
    HAL_GPIO_WritePin(AMP_EN_PORT, AMP_EN_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void audio_write(uint16_t level) {
    HAL_DAC_SetValue(&dac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, level & 0x0FFF);
}
