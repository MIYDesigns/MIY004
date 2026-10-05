#include "flash.h"
#include "pins.h"
#include "stm32l4xx_hal.h"

static SPI_HandleTypeDef spi;

static void cs(bool active) {
    HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, active ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void xfer(const uint8_t *tx, uint8_t *rx, uint16_t len) {
    cs(true);
    HAL_SPI_TransmitReceive(&spi, (uint8_t *)tx, rx, len, 10);
    cs(false);
}

void flash_power(bool on) {
    HAL_GPIO_WritePin(STOR_IN_PORT, STOR_IN_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    if (on) {
        HAL_Delay(1);  // tPU: VCC(min) to first command
    }
}

void flash_init(void) {
    __HAL_RCC_SPI2_CLK_ENABLE();

    // CS idles high before the rail comes up
    HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
    GPIO_InitTypeDef g = {0};
    g.Pin = FLASH_CS_PIN;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FLASH_CS_PORT, &g);

    g.Pin = FLASH_SCK_PIN | FLASH_MISO_PIN | FLASH_MOSI_PIN;
    g.Mode = GPIO_MODE_AF_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(FLASH_SPI_PORT, &g);

    spi.Instance = SPI2;
    spi.Init.Mode = SPI_MODE_MASTER;
    spi.Init.Direction = SPI_DIRECTION_2LINES;
    spi.Init.DataSize = SPI_DATASIZE_8BIT;
    spi.Init.CLKPolarity = SPI_POLARITY_LOW;   // mode 0
    spi.Init.CLKPhase = SPI_PHASE_1EDGE;
    spi.Init.NSS = SPI_NSS_SOFT;
    spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  // 6 MHz at 48 MHz PCLK1 - bring-up safe
    spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi.Init.TIMode = SPI_TIMODE_DISABLE;
    spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    HAL_SPI_Init(&spi);

    flash_power(true);
}

flash_id_t flash_read_id(void) {
    uint8_t tx[4] = {0x9F, 0, 0, 0};
    uint8_t rx[4] = {0};
    xfer(tx, rx, sizeof tx);
    return (flash_id_t){rx[1], rx[2], rx[3]};
}

bool flash_id_ok(flash_id_t id) {
    return id.mfr == FLASH_JEDEC_MFR && id.type == FLASH_JEDEC_TYPE &&
           id.capacity == FLASH_JEDEC_CAPACITY;
}

static uint8_t read_reg(uint8_t cmd) {
    uint8_t tx[2] = {cmd, 0};
    uint8_t rx[2] = {0};
    xfer(tx, rx, sizeof tx);
    return rx[1];
}

uint8_t flash_read_sr1(void) {
    return read_reg(0x05);
}

uint8_t flash_read_cr1(void) {
    return read_reg(0x35);
}
