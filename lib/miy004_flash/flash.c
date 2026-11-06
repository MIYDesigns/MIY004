#include "flash.h"
#include "pins.h"
#include "stm32l4xx_hal.h"

// S25FL256L commands (datasheet 002-00124). Array access always uses the 4-byte-address
// opcodes, so the result does not depend on CR2V[ADS] and addresses >= 16 MB never wrap.
#define CMD_RDID   0x9F
#define CMD_RDSR1  0x05
#define CMD_RDSR2  0x07
#define CMD_RDCR1  0x35
#define CMD_RDCR2  0x15
#define CMD_RDCR3  0x33
#define CMD_WREN   0x06
#define CMD_CLSR   0x30
#define CMD_4READ  0x13
#define CMD_4PP    0x12
#define CMD_4SE    0x21
#define CMD_RES    0xAB
#define CMD_MBR    0xFF
#define CMD_RSTEN  0x66
#define CMD_RST    0x99

#define SR1_WIP    (1u << 0)
#define SR2_P_ERR  (1u << 5)
#define SR2_E_ERR  (1u << 6)

static SPI_HandleTypeDef spi;
static bool powered;
// MEDIUM: on BREAKOUTS jumpers HIGH/VERY_HIGH edges ring on SCK and the flash counts extra
// clocks (bit-slipped page reads at 6-24 MHz); LOW is too slow for 24 MHz. MEDIUM read
// 60/60 pages clean at 6, 12 and 24 MHz (DEV-109, 2026-11-06).
static uint32_t sck_speed = GPIO_SPEED_FREQ_MEDIUM;

static void cs(bool active) {
    HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, active ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void xfer(const uint8_t *tx, uint8_t *rx, uint16_t len) {
    cs(true);
    HAL_SPI_TransmitReceive(&spi, (uint8_t *)tx, rx, len, 10);
    cs(false);
}

// While VFLASH is off every line the MCU could drive (CS#, SCK, SI) is left analog / high-Z:
// driving any of them high into an unpowered flash back-feeds it through its ESD diodes
// (the same reason R3-R5 pull up to VFLASH, not 3V3). SO is an MCU input either way.
static void bus_release(void) {
    GPIO_InitTypeDef g = {0};
    g.Pin = FLASH_CS_PIN;
    g.Mode = GPIO_MODE_ANALOG;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FLASH_CS_PORT, &g);
    g.Pin = FLASH_SCK_PIN | FLASH_MISO_PIN | FLASH_MOSI_PIN;
    HAL_GPIO_Init(FLASH_SPI_PORT, &g);
}

static void bus_claim(void) {
    HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);  // deselected first
    GPIO_InitTypeDef g = {0};
    g.Pin = FLASH_CS_PIN;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(FLASH_CS_PORT, &g);

    g.Pin = FLASH_SCK_PIN | FLASH_MISO_PIN | FLASH_MOSI_PIN;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = sck_speed;
    g.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(FLASH_SPI_PORT, &g);
}

void flash_set_pin_speed(uint32_t gpio_speed) {
    sck_speed = gpio_speed;
    if (powered) {
        bus_claim();
    }
}

static void cmd_only(uint8_t cmd);

// Return the part to a known state: Mode Bit Reset (FFh) first, so a device left in continuous
// read mode will hear the next command, then software reset (RSTEN 66h + RST 99h) and tRPH
// (100 us). Needed whenever the MCU restarts without VFLASH cycling, or after the bus may have
// been corrupted (a bit-slipped command can leave a one-shot mode armed).
void flash_reset(void) {
    // RES (ABh) first: in deep power-down the part ignores everything else, including MBR and
    // RST. Harmless when not powered down. tRES 5 us, covered by the delay below. A bit-slipped
    // command decoded as DPD (B9h) left the part dead to MBR/RST on BREAKOUTS (DEV-109).
    cmd_only(CMD_RES);
    for (volatile int i = 0; i < 500; i++) {
    }
    cmd_only(CMD_MBR);
    cmd_only(CMD_RSTEN);
    cmd_only(CMD_RST);
    HAL_Delay(1);
}

void flash_power(bool on) {
    if (on) {
        HAL_GPIO_WritePin(STOR_IN_PORT, STOR_IN_PIN, GPIO_PIN_SET);
        // tPU max 300 us from VCC(min) to first command; HAL_Delay(1) waits 1-2 ms, which also
        // covers the VFLASH switch's own rise time on MAIN-PCB.
        HAL_Delay(1);
        bus_claim();
        powered = true;
        flash_reset();
    } else {
        bus_release();
        HAL_GPIO_WritePin(STOR_IN_PORT, STOR_IN_PIN, GPIO_PIN_RESET);
    }
    powered = on;
}

void flash_init(void) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    bus_release();

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
    // HAL leaves SPE=0 until the first transfer, and with SPE=0 the AF pins are not driven:
    // SCK would float (no pull) until after CS# is already low. Enable now so SCK sits at its
    // mode-0 idle (low) before any command. Seen as a rare bad JEDEC read (DEV-109).
    __HAL_SPI_ENABLE(&spi);

    flash_power(true);
}

void flash_set_prescaler(uint32_t spi_baudrate_prescaler) {
    __HAL_SPI_DISABLE(&spi);
    spi.Init.BaudRatePrescaler = spi_baudrate_prescaler;
    MODIFY_REG(spi.Instance->CR1, SPI_CR1_BR, spi_baudrate_prescaler);
    __HAL_SPI_ENABLE(&spi);  // drive SCK to idle again before the next CS# assert
}

flash_id_t flash_read_id(void) {
    uint8_t tx[4] = {CMD_RDID, 0, 0, 0};
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

uint8_t flash_read_sr1(void) { return read_reg(CMD_RDSR1); }
uint8_t flash_read_sr2(void) { return read_reg(CMD_RDSR2); }
uint8_t flash_read_cr1(void) { return read_reg(CMD_RDCR1); }
uint8_t flash_read_cr2(void) { return read_reg(CMD_RDCR2); }
uint8_t flash_read_cr3(void) { return read_reg(CMD_RDCR3); }

static void cmd_only(uint8_t cmd) {
    uint8_t rx;
    xfer(&cmd, &rx, 1);
}

static void put_addr4(uint8_t *p, uint8_t cmd, uint32_t addr) {
    p[0] = cmd;
    p[1] = (uint8_t)(addr >> 24);
    p[2] = (uint8_t)(addr >> 16);
    p[3] = (uint8_t)(addr >> 8);
    p[4] = (uint8_t)addr;
}

// Poll WIP. On P_ERR/E_ERR the part stays busy until CLSR, so clear it and report.
static flash_status_t wait_ready(uint32_t timeout_ms) {
    uint32_t start = HAL_GetTick();
    for (;;) {
        uint8_t sr1 = flash_read_sr1();
        if (!(sr1 & SR1_WIP)) {
            return FLASH_OK;
        }
        uint8_t sr2 = flash_read_sr2();
        if (sr2 & (SR2_P_ERR | SR2_E_ERR)) {
            cmd_only(CMD_CLSR);
            return (sr2 & SR2_E_ERR) ? FLASH_E_ERR : FLASH_P_ERR;
        }
        if (HAL_GetTick() - start > timeout_ms) {
            return FLASH_TIMEOUT;
        }
    }
}

flash_status_t flash_read(uint32_t addr, uint8_t *buf, uint32_t len) {
    if (!powered) return FLASH_BAD_ARG;
    if (addr >= FLASH_SIZE_BYTES || len > FLASH_SIZE_BYTES - addr) return FLASH_BAD_ARG;
    uint8_t hdr[5];
    put_addr4(hdr, CMD_4READ, addr);
    cs(true);
    HAL_SPI_Transmit(&spi, hdr, sizeof hdr, 10);
    while (len) {
        uint16_t n = len > 0xFFFF ? 0xFFFF : (uint16_t)len;
        HAL_SPI_Receive(&spi, buf, n, 10 + n / 64);
        buf += n;
        len -= n;
    }
    cs(false);
    return FLASH_OK;
}

flash_status_t flash_program_page(uint32_t addr, const uint8_t *data, uint32_t len) {
    if (!powered) return FLASH_BAD_ARG;
    // Must not cross a 256-byte page: the part wraps within the page instead.
    if (len == 0 || addr >= FLASH_SIZE_BYTES ||
        (addr % FLASH_PAGE_BYTES) + len > FLASH_PAGE_BYTES) {
        return FLASH_BAD_ARG;
    }
    cmd_only(CMD_WREN);
    uint8_t hdr[5];
    put_addr4(hdr, CMD_4PP, addr);
    cs(true);
    HAL_SPI_Transmit(&spi, hdr, sizeof hdr, 10);
    HAL_SPI_Transmit(&spi, (uint8_t *)data, (uint16_t)len, 10);
    cs(false);
    return wait_ready(10);  // tPP max 1.2 ms
}

flash_status_t flash_erase_sector(uint32_t addr) {
    if (!powered) return FLASH_BAD_ARG;
    if (addr >= FLASH_SIZE_BYTES) return FLASH_BAD_ARG;
    cmd_only(CMD_WREN);
    uint8_t hdr[5];
    put_addr4(hdr, CMD_4SE, addr & ~(FLASH_SECTOR_BYTES - 1));
    uint8_t rx[5];
    xfer(hdr, rx, sizeof hdr);
    return wait_ready(500);  // tSE max 250 ms
}
