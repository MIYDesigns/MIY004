#pragma once
// Infineon S25FL256L (U2), 32 MB SPI-NOR on SPI2.
#include <stdbool.h>
#include <stdint.h>

#define FLASH_JEDEC_MFR      0x01  // Infineon / Cypress
#define FLASH_JEDEC_TYPE     0x60
#define FLASH_JEDEC_CAPACITY 0x19  // 256 Mbit

#define FLASH_CR1_QUAD       (1u << 1)  // QE lives in CR1 on this part, not SR1

typedef struct {
    uint8_t mfr;
    uint8_t type;
    uint8_t capacity;
} flash_id_t;

// Powers VFLASH (STOR_IN) and configures SPI2. Call after miy_board_init().
void flash_init(void);
void flash_power(bool on);

flash_id_t flash_read_id(void);  // RDID 0x9F
bool flash_id_ok(flash_id_t id);
uint8_t flash_read_sr1(void);    // RDSR1 0x05
uint8_t flash_read_cr1(void);    // RDCR1 0x35
