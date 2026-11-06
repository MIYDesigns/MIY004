#pragma once
// Infineon S25FL256L (U2), 32 MB SPI-NOR on SPI2.
#include <stdbool.h>
#include <stdint.h>

#define FLASH_JEDEC_MFR      0x01  // Infineon / Cypress
#define FLASH_JEDEC_TYPE     0x60
#define FLASH_JEDEC_CAPACITY 0x19  // 256 Mbit

#define FLASH_SIZE_BYTES     (32u * 1024u * 1024u)
#define FLASH_SECTOR_BYTES   4096u
#define FLASH_PAGE_BYTES     256u

#define FLASH_CR1_QUAD       (1u << 1)  // QE lives in CR1 on this part, not SR1
#define FLASH_CR2_ADS        (1u << 0)  // CR2V[0]: 1 = 4-byte addressing for the legacy opcodes

typedef struct {
    uint8_t mfr;
    uint8_t type;
    uint8_t capacity;
} flash_id_t;

typedef enum {
    FLASH_OK = 0,
    FLASH_TIMEOUT,
    FLASH_P_ERR,    // program failed (SR2V P_ERR), cleared with CLSR
    FLASH_E_ERR,    // erase failed (SR2V E_ERR), cleared with CLSR
    FLASH_BAD_ARG,  // out of range, crosses a page, or VFLASH is off
} flash_status_t;

// Configures SPI2 with the bus released, then powers VFLASH (STOR_IN) and claims the bus.
// Call after miy_board_init().
void flash_init(void);
// on: VFLASH up, wait tPU, then drive CS# high and hand SCK/SI to SPI2.
// off: release CS#/SCK/SI to high-Z first, then VFLASH down (no back-feed).
void flash_power(bool on);
// Mode Bit Reset + software reset (FFh, 66h, 99h) + tRPH. flash_power(true) already does this.
void flash_reset(void);
// SPI_BAUDRATEPRESCALER_x; PCLK1 is 48 MHz. Single-lane READ is rated to 50 MHz.
void flash_set_prescaler(uint32_t spi_baudrate_prescaler);
// GPIO_SPEED_FREQ_x for SCK/SI/SO (slew rate). Slower edges ring less on long wiring.
void flash_set_pin_speed(uint32_t gpio_speed);

flash_id_t flash_read_id(void);  // RDID 0x9F
bool flash_id_ok(flash_id_t id);
uint8_t flash_read_sr1(void);    // RDSR1 0x05
uint8_t flash_read_sr2(void);    // RDSR2 0x07 (P_ERR bit 5, E_ERR bit 6)
uint8_t flash_read_cr1(void);    // RDCR1 0x35
uint8_t flash_read_cr2(void);    // RDCR2 0x15 (ADS bit 0)
uint8_t flash_read_cr3(void);    // RDCR3 0x33

// Array access, always 4-byte-address opcodes (4READ 13h, 4PP 12h, 4SE 21h).
flash_status_t flash_read(uint32_t addr, uint8_t *buf, uint32_t len);
flash_status_t flash_program_page(uint32_t addr, const uint8_t *data, uint32_t len);  // within one 256 B page
flash_status_t flash_erase_sector(uint32_t addr);  // 4 KB sector containing addr
