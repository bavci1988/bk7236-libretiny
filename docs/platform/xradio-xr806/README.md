# XRadio XR806

## Introduction

XRadio XR806 is a Wi-Fi and BLE microcontroller by XRadio Technology (Allwinner). It is used e.g. in the Tuya WXU module.

Features:

- 32-bit ARMv8-M CPU (Cortex-M33 compatible, 160 MHz)
- 300 KiB SRAM for the application
- 2 MiB flash with XiP
- 802.11b/g/n Wi-Fi
- Bluetooth LE (not supported by LibreTiny)

Resources:

- [XR806 SDK](https://github.com/XradioTech/xr806_sdk) : Official XR806 SDK
- [XR806 documentation](https://xr806.docs.aw-ol.com/) : Official XR806 documentation
- [Tuya WXU](https://developer.tuya.com/en/docs/iot/wxu-module-datasheet?id=Kc2xk9qlk04so) : Tuya WXU module datasheet

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

---

## Flashing

{%
	include-markdown "../../inc/flashing-note.md"
%}

XR806 has three UART ports. UART0 is used for flashing; UART2 is used for text output (logs).

!!! note
	On Tuya WXU modules, UART0 is also the UART of the device's MCU (e.g. a thermostat or a dimmer controller).
	Disconnect or hold the MCU in reset while flashing.

### Wiring

Connect UART0 of the XR806 to the USB-TTL adapter:

PC  | XR806
----|-----------------------
RX  | TX0 (PB00)
TX  | RX0 (PB01)
GND | GND

On Tuya WXU modules, TX0 is pad 15, RX0 is pad 16 and PB02 is pad 17.

{%
	include-markdown "../../inc/uart-power.md"
%}

The download mode is entered when PB02 is held low while the chip is powered on or reset. Hence, the first step is running the flasher program (described below). While the program is trying to establish communication, connect PB02 to GND and power-cycle the chip (or briefly short CHIP_PWD to GND). Disconnect PB02 from GND afterwards.

A running SDK application may also enter download mode by itself, when it receives the `upgrade` command, which the flasher sends first.

### Partition layout

{%
	include-markdown "../../inc/partition-layout.md"
%}

XR806 uses the compressed OTA scheme of the SDK:

- the bootloader is at 0x0, followed by the application partition at 0x8000; the application partition holds the
  whole image chain (app, app_xip, wlan_bl, wlan_fw, sys_sdd)
- the OTA configuration (boot selection) is at 0x13F000
- an OTA update writes the XZ-compressed image into the download partition at 0x140000; on the next boot, the
  bootloader unpacks it over the application partition

The image is checked fully (including decompression) before the update is committed. There is no rollback: if the
new firmware doesn't work, it has to be flashed over UART.

### Flashing

The recommended tool to flash (or dump firmware) is `ltchiptool`.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! tip
	Because the UART uploading code is programmed in the ROM of the chip, it can't be software-bricked, even if you damage the bootloader.

## Firmware output files

These files are present in the build directory after successful compilation:

File                            | Description
--------------------------------|----------------------------------------
**firmware.uf2**                | **UF2 package for UART and OTA upload**
**image_firmware.0x000000.img** | **Full flash binary image** - flashable at 0x0
image_boot.0x000000.bin         | Boot partition binary image - flashable at 0x0
image_app.0x008000.bin          | Application partition binary image - flashable at 0x8000
image_ota.0x140000.bin          | XZ-compressed OTA image - written to the download partition at 0x140000

## Power management

Besides the LibreTiny API, the family core has `xr806_pm.h`, with standby (all RAM retained, wake-up by timer,
a GPIO or received Wi-Fi data), PM wakelocks, the Wi-Fi DTIM period and listen interval, and the UART state for an
idle check. Deep sleep (`lt_deep_sleep_*()`) is not supported: in tests, the chip did not wake up from hibernation.

### Power design

The chip makes its core voltage with an internal LDO (e.g. Tuya WXU) or with an internal DC-DC converter (an inductor
at pin VDD_LX). The firmware and the bootloader must match the board, or the chip stops at boot with
`Power config conflict!`. The boards default to the LDO; for a board with the DC-DC converter, set
`board_build.xr806_power = dcdc` in `platformio.ini` (in ESPHome: `power_supply: dcdc` under `xr806:`). This also selects the matching bootloader for the full flash
image. An OTA update does not change the bootloader.
