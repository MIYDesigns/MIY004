# MIY004 firmware

Firmware for the M.I.Y. Designs MIY004 audio fidget spinner: spin it, and the
rotation rate drives the volume, with a fast attack and slow decay.

**Status:** early bring-up. Board support, pin map and drivers for the flash, DAC and
Hall sensors are in place; playback and the spin-rate envelope are not wired up yet.

## Hardware

| | |
|---|---|
| MCU | STM32L452CEU6 (UFQFPN48, 512 KB flash, 160 KB RAM) |
| Storage | Infineon S25FL256L, 32 MB SPI-NOR on SPI2 |
| Amplifier | TI TPA2005D1 class-D, driven from DAC1_OUT1 |
| Rate sensor | TI DRV5011 Hall latch |
| Wake sensor | TI DRV5032FB Hall switch |
| Clock | No crystal. USB runs from HSI48, trimmed by the CRS against USB start-of-frame |

## Build

Requires [PlatformIO](https://platformio.org/).

```sh
pio run                  # build
pio run -t upload        # flash over ST-LINK (SWD)
pio test -e native       # host-side unit tests, no hardware needed
```

## Layout

```
boards/        PlatformIO board definition for the STM32L452CEU6
lib/
  miy004_board/     pin map, clocks (HSI48 + CRS), interrupt handlers
  miy004_flash/     S25FL256L driver
  miy004_audio/     DAC output and amplifier enable
  miy004_hall/      Hall sensor access
  miy004_envelope/  spin-rate to volume envelope (hardware-free, unit-tested)
src/           application
test/          unit tests for the native environment
```
