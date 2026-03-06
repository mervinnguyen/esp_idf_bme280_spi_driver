
#include "bme280.hpp"

// ===== ESP-IDF SDK Headers =====
// spi_common.h: Common SPI driver types and functions
#include "driver/spi_common.h"
// spi_types.h: Hardware-specific SPI type definitions
#include "hal/spi_types.h"
// soc.h: System-on-Chip register and peripheral definitions
#include "soc/soc.h"

/**
 * @brief BME280 Constructor - Initializes the BME280 sensor with SPI interface
 * 
 * Sets up the SPI bus and device, then acquires calibration coefficients from
 * the sensor's non-volatile memory. This constructor ensures the sensor is
 * ready for measurement operations upon completion.
 * 
 * @param devcfg SPI device interface configuration (clock speed, mode, CS pin, etc.)
 * @param bus_config SPI bus configuration (MISO, MOSI, CLK pin assignments)
 * @param spi_dev Reference to SPI device handle (populated by spi_bus_add_device)
 * 
 * @note Uses VSPI_HOST (SPI3) on ESP32 with DMA channel 1 for data transfers
 */
BME280::BME280(const spi_device_interface_config_t &devcfg, const spi_bus_config_t &bus_config, spi_device_handle_t &spi_dev) : devcfg{ devcfg }, buscfg{busconfig}, spi_dev{spi_dev} 
{
    // Initialize the SPI bus with provided configuration on VSPI_HOST (SPI3)
    // DMA channel is set to 1 for efficient DMA-based transfers
    // This must be called before adding any devices to the bus
    spi_bus_initialize(VSPI_HOST, &buscfg, 1u);
    
    // Register the BME280 device on the initialized SPI bus
    // This populates the spi_dev handle for all subsequent SPI transactions
    // The device is now ready to accept read/write commands
    spi_bus_add_device(VSPI_HOST, &devcfg, &spi_dev);

    // Retrieve calibration coefficients stored in sensor's factory-programmed NVM
    // These unique-per-sensor values are essential for converting raw readings
    // to accurate temperature, pressure, and humidity measurements
    get_calibration_data();
}

/**
 * @brief Retrieve calibration coefficients from BME280 non-volatile memory
 * 
 * Reads all factory-calibrated coefficients from the BME280 sensor and stores
 * them in the calibration_data structure. These coefficients are essential for
 * the compensation formulas that convert raw sensor readings to actual values.
 * 
 * The BME280 stores separate calibration coefficients for:
 * - Temperature (T1, T2, T3)
 * - Pressure (P1-P9)
 * - Humidity (H1-H6)
 * 
 * Some humidity coefficients (H4, H5) require bit-level unpacking due to
 * their non-byte-aligned storage in the sensor's memory map.
 */
void BME280::get_calibration_data(void){
    // ===== Temperature Calibration Coefficients =====
    // Three 16-bit signed coefficients used in temperature compensation formula
    register_read(ADDR_DIG_T1, (uint8_t*)&(calibration_data.dig_T1), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_T2, (uint8_t*)&(calibration_data.dig_T2), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_T3, (uint8_t*)&(calibration_data.dig_T3), CAL_SIZE_2_BYTE);
    
    // ===== Pressure Calibration Coefficients =====
    // Nine coefficients (P1-P9) for pressure compensation calculations
    // P1 is 16-bit unsigned; P2 and P3 are signed coefficients
    // P4-P9 are additional fine-tuning coefficients for accuracy
    register_read(ADDR_DIG_P1, (uint8_t*)&(calibration_data.dig_P1), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P2, (uint8_t*)&(calibration_data.dig_P2), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P3, (uint8_t*)&(calibration_data.dig_P3), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P4, (uint8_t*)&(calibration_data.dig_P4), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P5, (uint8_t*)&(calibration_data.dig_P5), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P6, (uint8_t*)&(calibration_data.dig_P6), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P7, (uint8_t*)&(calibration_data.dig_P7), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P8, (uint8_t*)&(calibration_data.dig_P8), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P9, (uint8_t*)&(calibration_data.dig_P9), CAL_SIZE_2_BYTE);
    
    // ===== Humidity Calibration Coefficients =====
    // Simple coefficients: H1 and H3 are 8-bit, H2 is 16-bit signed
    register_read(ADDR_DIG_H1, (uint8_t*)&(calibration_data.dig_H1), CAL_SIZE_1_BYTE);
    register_read(ADDR_DIG_H2, (uint8_t*)&(calibration_data.dig_H2), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_H3, (uint8_t*)&(calibration_data.dig_H3), CAL_SIZE_1_BYTE);

    // H4 and H5: Require special bit unpacking - these coefficients span across
    // byte boundaries in the sensor's memory, necessitating bit manipulation
    register_read(ADDR_DIG_H4, (uint8_t*)&(calibration_data.dig_H4), CAL_SIZE_2_BYTE);
    // Extract upper 4 bits from high byte and lower 8 bits from low byte, then repack
    calibration_data.dig_H4 = ((calibration_data.dig_H4 >> 8u) & 0xF) |
                              ((calibration_data.dig_H4 & 0xFF) << 4u);

    // H5 coefficient with similar cross-byte storage
    register_read(ADDR_DIG_H5, (uint8_t*)&(calibration_data.dig_H5), CAL_SIZE_2_BYTE);
    // Extract lower 4 bits from low byte and upper 8 bits from high byte, then realign
    calibration_data.dig_H5 = (((calibration_data.dig_H5 & 0xF) << 4u) |
                                (calibration_data.dig_H5 & 0xFF00)) >> 4u;

    // H6: Final 8-bit humidity coefficient
    register_read(ADDR_DIG_H6, (uint8_t*)&(calibration_data.dig_H6), CAL_SIZE_1_BYTE);
}

/**
 * @brief Reset all BME280 control registers to their default state
 * 
 * Writes reset values (0x00) to the three main control registers:
 * - CTRL_MEAS: Controls measurement mode, oversampling, and operation mode
 * - CONFIG: Sets filter coefficient and standby duration
 * - CTRL_HUM: Controls humidity oversampling settings
 * 
 * This method performs a software reset via register writes to ensure the
 * sensor returns to a known idle state. Useful for recovery and re-initialization.
 * 
 * @note This is a register-level reset, not a hardware reset via the reset pin
 */
void BME280::clear_all_registers(void){
    // ===== Prepare Transmission Buffer =====
    // Send pairs of (register address, value) where register is write-only
    // Each write operation is: [address with write bit, data byte]
    // Total of 6 bytes: 3 registers × (address + data)
    uint8_t tx_buffer[6u] = 
    {
        // First register: CTRL_MEAS with write-only mask applied to address
        CTRL_MEAS & REG_WRITE_ONLY,
        // Value to write: 0x00 (puts sensor into sleep mode, disables measurements)
        0x00u,
        
        // Second register: CONFIG with write-only mask
        CONFIG & REG_WRITE_ONLY,
        // Value to write: 0x00 (sets filter off, no standby delay)
        0x00u,
        
        // Third register: CTRL_HUM with write-only mask
        CTRL_HUM & REG_WRITE_ONLY,
        // Value to write: 0x00 (humidity oversampling disabled)
        0x00u,
    };

    // ===== Configure SPI Transaction =====
    // Structure defining the SPI transaction parameters for this operation
    spi_transaction_t trans =
    {
        // Flags: No special flags needed for this write operation
        .flags = 0,
        // Command phase: Not used in this implementation
        .cmd = 0,
        // Address phase: Not used in this implementation
        .addr = 0,
        // Length: 6 bytes × 8 bits = 48 bits of data to transmit
        .length = 8u * 6u,
        // RX length: 0 - This is a write-only operation (no read back)
        .rxlength = 0u,
        // User data: NULL - No context data needed for this transaction
        .user = NULL,
        // Transmit buffer: Points to our tx_buffer containing addresses and values
        .tx_buffer = &tx_buffer,
        // Receive buffer: NULL - We don't expect any response data
        .rx_buffer = NULL,
    };
    
    // ===== Execute SPI Transaction =====
    // Acquire exclusive access to the SPI bus
    // portMAX_DELAY ensures we wait indefinitely if needed (RTOS context)
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);

    // Transmit the register reset commands to the BME280 sensor
    spi_device_transmit(spi_dev, &trans);

    // Release the SPI bus so other devices can use it
    // Critical for systems with multiple SPI devices on the same bus
    spi_device_release_bus(spi_dev);
}