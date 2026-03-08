# ESP-IDF BME280 SPI Driver (C++)

## Overview

A production-ready C++ driver implementation for the Bosch Sensortec BME280 environmental sensor, providing high-performance SPI communication through the ESP-IDF framework. This driver enables precision measurement of temperature, barometric pressure, and relative humidity with full support for the sensor's operational modes.

The architecture prioritizes modularity, performance, and maintainability, offering a clean abstraction layer over the ESP-IDF SPI master interface while maintaining minimal memory footprint and deterministic behavior suitable for embedded systems.

---

## Key Features

- **Native SPI Integration**: Leverages ESP-IDF `spi_master` driver for efficient hardware communication
- **Modern C++ Implementation**: Type-safe, RAII-compliant design with minimal runtime overhead
- **Full Environmental Sensing**:
  - Temperature measurement with configurable resolution
  - Barometric pressure sensing with oversampling support
  - Relative humidity monitoring
- **Flexible Operating Modes**:
  - Forced Mode: Single-shot measurements with power optimization
  - Normal Mode: Continuous sampling with configurable standby periods
- **Architectural Design**:
  - Clear separation of concerns between driver abstraction and application logic
  - Extensible class hierarchy for custom measurement profiles
  - Thread-safe operation with proper resource management
- **Platform Compatibility**: Validated across ESP32, ESP32-S3, and ESP32-C3 variants

---

## Hardware Requirements

| Component | Specification |
|-----------|--------------|
| **Sensor** | Bosch Sensortec BME280 |
| **MCU Platform** | ESP32 series (ESP32, ESP32-S3, ESP32-C3, etc.) |
| **Communication Interface** | 4-wire SPI (up to 10 MHz) |
| **Framework** | ESP-IDF v4.4+ |

---

## ESP32 SPI Pinout Reference

Below is the ESP32 SPI pin configuration used in this project.

![ESP32 SPI Pinout](manuals/esp32_pinout.jpg)

> Ensure MOSI, MISO, SCLK, and CS are correctly mapped in your `spi_bus_config_t` and `spi_device_interface_config_t` structures inside ESP-IDF.

---

## BME280 Datasheet

For full register definitions, compensation formulas, timing diagrams, and electrical characteristics, refer to the official Bosch datasheet:

📄 **[Download BME280 Datasheet](manuals/bme280_datasheet.pdf)**

This driver implementation follows:
- Register map definitions
- Calibration parameter extraction
- Compensation algorithms for temperature, pressure, and humidity
- Timing constraints and SPI protocol requirements

---

## Notes

- Default SPI mode: Mode 0 (CPOL = 0, CPHA = 0)
- Maximum tested SPI clock: 10 MHz
- Ensure proper pull-ups on CS and correct power sequencing (VDD/VDDIO)

---
