# pico2w_bt_diagnosis

[中文](README_ZH.md)

A minimal Raspberry Pi Pico 2 W Bluetooth Classic inquiry diagnostic firmware.

This project powers on the Pico 2 W CYW43 Bluetooth controller, starts a Bluetooth inquiry scan, prints nearby device information over USB serial, and repeats the scan every few seconds. It is intended as a small diagnosis program for checking whether Bluetooth initialization and inquiry scanning work on a Pico 2 W board.

## What It Prints

The USB serial log includes:

- Board and Pico SDK information
- CYW43 initialization status
- Bluetooth ready state
- Inquiry scan rounds
- Discovered Bluetooth device address
- Class of device
- RSSI, when available
- Device name, when available

## Requirements

- Raspberry Pi Pico 2 W
- Raspberry Pi Pico SDK 2.2.0 or compatible
- CMake and Ninja
- A USB serial monitor

The project is configured for:

```cmake
set(PICO_BOARD pico2_w CACHE STRING "Board type")
```

## Build

From the project directory:

```powershell
cmake --build .\build
```

The main UF2 output is:

```text
build/pico2w_bt_diagnosis.uf2
```

If the build directory has not been configured yet, configure it first with your Pico SDK environment loaded:

```powershell
cmake -S . -B build -G Ninja
cmake --build .\build
```

## Flash

Hold the Pico 2 W BOOTSEL button while plugging it into USB, then copy:

```text
build/pico2w_bt_diagnosis.uf2
```

to the mounted `RP2350` drive.

## Serial Output

After flashing, open the Pico USB serial port. The firmware waits briefly for USB serial, then continues running even if no serial monitor is connected.

Example output:

```text
pico2w_bt_diagnosis
Board: pico2_w
Pico SDK: 2.2.0
CYW43 init status: 0
Powering on Bluetooth...
Bluetooth ready

Scan 1 started
[1] device 1: addr=AA:BB:CC:DD:EE:FF, cod=0x240404, rssi=-52 dBm, name="Device"
Scan 1 complete: 1 device(s)
Next scan in 5000 ms.
```

## Notes

- USB serial is enabled and UART stdio is disabled.
- The firmware uses Bluetooth Classic inquiry through BTstack.
- This is a diagnosis firmware, not a Bluetooth pairing or HID connection example.
