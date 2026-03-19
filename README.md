# ESP-IDF BME280 SPI Driver

ESP32 C++ driver for Bosch BME280 over SPI, built with ESP-IDF.

This project includes:
- Embedded firmware that reads temperature, pressure, and humidity from BME280
- Host-side GoogleTest unit tests for compensation math and buffer-safety logic

## Features

- SPI communication using ESP-IDF spi_master
- Temperature, pressure, and humidity compensation
- Forced-mode and normal-mode support in driver API
- Host tests for compensation formulas and transfer safety checks

## Repository Layout

- main/: Firmware source code
  - bme280.hpp / bme280.cpp: Sensor driver
  - bme280_algorithms.hpp / bme280_algorithms.cpp: Pure algorithm layer
  - spi.cpp: App entry, SPI config, and sensor task
- tests/: Host-side GoogleTest project
- Wiring_Diagram.md: Pin mapping and wiring details

## Hardware

- ESP32 development board
- BME280 sensor module (SPI-capable)
- Jumper wires

Default pins used in this project (defined in main/spi.cpp):
- MISO: GPIO19
- MOSI: GPIO23
- SCLK: GPIO18
- CS: GPIO5

See Wiring_Diagram.md for full wiring instructions.

## Firmware Behavior

Current app_main starts task_normal_mode in main/spi.cpp.

Normal mode configuration:
- Oversampling: 16x for temperature, pressure, humidity
- Output period: 5 seconds
- UART output example:
  - Temperature: xx.xx C
  - Pressure: xxxx.xx hPa
  - Humidity: xx.xx %

## Prerequisites

- ESP-IDF installed (this repo has been used with ESP-IDF v6.1)
- Python environment required by ESP-IDF
- USB serial access to ESP32 for flashing/monitoring

## Build (Linux/WSL)

From repo root:

1) Load ESP-IDF environment
source ~/esp/esp-idf/export.sh

2) Build
idf.py build

## Flash and Monitor

If serial device is visible in Linux/WSL:

idf.py -p /dev/ttyUSB0 flash monitor

Replace /dev/ttyUSB0 with your device (sometimes /dev/ttyACM0).

### WSL Note

If no serial ports appear in WSL, this is expected without USB passthrough.

Use one of these approaches:
- Flash from Windows terminal using ESP-IDF/esptool
- Attach USB to WSL with usbipd-win, then flash from WSL

## Host Unit Tests (GoogleTest)

These tests validate logic without hardware.

Covered areas:
- Temperature compensation
- Pressure compensation
- Humidity compensation and clamping
- Transfer-size validation
- Safe payload copy behavior

Run tests:

cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure

## Troubleshooting

1) idf.py: command not found
- Source ESP-IDF export script first:
  source ~/esp/esp-idf/export.sh

2) No serial ports found when flashing
- Check cable/data support and board power
- Check permissions/groups on Linux
- If using WSL, use USB passthrough or flash from Windows

3) Build succeeds but no sensor data
- Verify wiring and sensor SPI mode
- Confirm BME280 module supports SPI (not I2C-only wiring)
- Check CS pin mapping and power (3.3V)

## References

- BME280 datasheet: manuals/bme280_datasheet.pdf
- ESP32 pinout image: manuals/esp32_pinout.jpg
