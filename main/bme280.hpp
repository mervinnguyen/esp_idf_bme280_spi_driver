/**
 * @file bme280.hpp
 * @brief BME280 Environmental Sensor Driver Interface
 * 
 * Defines the C++ class interface for communicating with the Bosch Sensortec
 * BME280 temperature, pressure, and humidity sensor via SPI. This header
 * provides a minimal abstraction layer over the ESP-IDF SPI master driver.
 * 
 * @note This is a minimal implementation focused on register access and
 *       basic initialization routines.
 */

#pragma once

#include "driver/spi_master.h"
#include <cstdint>

/**
 * @defgroup BME280_Registers BME280 Register Addresses
 * @{
 */

/** Chip ID register - contains fixed value 0x60 for BME280 */
#define BME280_REG_ID 0xD0

/** @} */

/**
 * @class BME280
 * @brief Driver class for BME280 environmental sensor over SPI interface
 * 
 * Provides an object-oriented interface to the BME280 sensor, managing
 * SPI communication through the ESP-IDF spi_master driver. This class
 * handles low-level register operations and ensures proper transaction
 * handling with the sensor hardware.
 * 
 * @note The SPI device must be initialized externally before passing the
 *       handle to this class constructor.
 */
class BME280 {
public:
	/**
	 * @brief Construct a new BME280 driver instance
	 * 
	 * Initializes the BME280 driver with a pre-configured SPI device handle.
	 * The SPI bus and device parameters (clock speed, mode, CS pin) must be
	 * configured prior to instantiating this object.
	 * 
	 * @param spi_handle Handle to an initialized ESP-IDF SPI device
	 * 
	 * @note Use of explicit prevents implicit conversion from spi_device_handle_t
	 */
	explicit BME280(spi_device_handle_t spi_handle);
	
	/**
	 * @brief Clear all BME280 configuration registers to default state
	 * 
	 * Performs a register-level reset of the BME280 sensor by writing default
	 * values to all writable configuration registers. This method is useful for
	 * ensuring a known device state during initialization or recovery procedures.
	 * 
	 * @note This is a software reset via register writes, not a hardware reset
	 */
	void clear_all_registers(void);

private:
	/** SPI device handle for hardware communication with the BME280 */
	spi_device_handle_t spi;
	
	/** 
	 * @brief Receive buffer for SPI transactions
	 * 
	 * Fixed-size buffer used to store data received from the BME280 during
	 * SPI read operations. Sized for typical register read transactions.
	 */
	uint8_t rx_data[4];
};
