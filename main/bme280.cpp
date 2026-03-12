
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
BME280::BME280(const spi_device_interface_config_t &devcfg, const spi_bus_config_t &bus_config, spi_device_handle_t &spi_dev) : dev_cfg{dev_cfg}, bus_cfg{bus_config}, spi_dev{spi_dev} 
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
    uint8_t tx_buffer[6] = 
    {
        // First register: CTRL_MEAS with write-only mask applied to address
        CTRL_MEAS & REG_WRITE_ONLY,
        // Value to write: 0x00 (puts sensor into sleep mode, disables measurements)
        0x00,
        
        // Second register: CONFIG with write-only mask
        CONFIG & REG_WRITE_ONLY,
        // Value to write: 0x00 (sets filter off, no standby delay)
        0x00,
        
        // Third register: CTRL_HUM with write-only mask
        CTRL_HUM & REG_WRITE_ONLY,
        // Value to write: 0x00 (humidity oversampling disabled)
        0x00,
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
        .length = 8 * 6,
        // RX length: 0 - This is a write-only operation (no read back)
        .rxlength = 0,
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

void BME280::register_write(uint8_t address, uint8_t data){
    //create a transmit buffer with register addressa and data (1 byte)
    uint8_t tx_buffer[2];   

    //Mask the address for write operations
    tx_buffer[0] = address | REG_READ_ONLY;

    //Send the data from sensor to buffer
    tx_buffer[1] = data;

    //Create SPI transaction structure
    spi_transaction_t trans;

    //clear structure fields
    memset(&trans, 0, sizeof(spi_transaction_t));

    //Number of bits to transmit (2 bytes = 16 bits)
    trans.length = 16;

    //Assign transmit buffer
    trans.tx_buffer = tx_buffer;

    //No receive buffer needed for writing to register
    trans.rx_buffer = 0;

    //Acquire SPI bus so other tasks can't access it
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);

    //Send transaction
    spi_device_transmit(spi_dev, &trans);

    //Release SPI bus
    spi_device_release_bus(spi_dev);
}

void BME280::register_read(const uint8_t address, const uint8_t size, uint8_t *buffer){
    //Create transmit and receive buffers
    uint8_t tx_buffer[MAX_TX_BUFFER_SIZE] = {0};
    uint8_t rx_buffer[MAX_RX_BUFFER_SIZE] = {0};

    //Set register address with read flag
    tx_buffer[0] = address | REG_READ_ONLY;

    //Create SPI transaction struct
    spi_transaction_t trans;

    //Clear the structure
    memset(&trans, 0, sizeof(spi_transaction_t));

    //Total bits to send and recieve (address + data)
    trans.length = 8 * (1 + size); // 1 byte for address + size bytes for data 
    trans.rxlength = 8 * (size + 1); // Only the data bytes are expected in response

    //Assign buffers
    trans.tx_buffer = tx_buffer;
    trans.rx_buffer = rx_buffer;

    //Acquire SPI bus for exclusive access
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);

    //Perform SPI transaction
    spi_device_transmit(spi_dev, &trans);

    //Release SPI bus
    spi_device_release_bus(spi_dev);

    //Copy received data (skip first byte which is dummuy)
    memcpy(buffer, &rx_buffer[1], size);
}

void BME280::burst_read_data(void){
    //First byte is the register addreess with read bit set
    uint8_t tx_buffer = PRESS_MSB | REG_READ_ONLY;

    //Buffer to receive 9 bytes (1 dummy + 8 data bytes)
    uint8_t rx_buffer[9];
    memset(rx_buffer, 0, sizeof(rx_buffer));

    //Declare trans structure
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));

    trans.length = 8 * 9; // 1 byte for address + 8 bytes for data
    trans.rxlength = 8 * 9; // 1 byte for address + 8 bytes for data
    trans.tx_buffer = &tx_buffer;       // Transmit the starting register address
    trans.rx_buffer = rx_buffer;        // Receive buffer for dummy byte + data bytes

    //Lock SPI bus
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);

    //Perform the transaction
    spi_device_transmit(spi_dev, &trans);

    //Release SPI bus
    spi_device_release_bus(spi_dev);

    //Combine presure bytes
    uint32_t press_msb = rx_buffer[1];
    uint32_t press_lsb = rx_buffer[2];
    uint32_t press_xlsb = rx_buffer[3];

    uint32_t raw_pressure = (press_msb << 16) | (press_lsb << 8) | (press_xlsb);

    raw_pressure >>= 4; // Pressure is 20 bits, so we need to shift right by 4 to align it

    //Combine temperature bytes
    uint32_t temp_msb = rx_buffer[4];
    uint32_t temp_lsb = rx_buffer[5];
    uint32_t temp_xlsb = rx_buffer[6];

    uint32_t raw_humidity = (hum_msb << 8) | hum_lsb;

    //Apply compensation
    int32_t temp_comp = compensate_T_int32(raw_temperature);
    int32_t press_comp = compensate_P_int64(raw_pressure);
    int32_t hum_comp = compensate_H_int32(raw_humidity);

    //Convert to floats
    temperature = temp_comp / 100.0f;        // Temperature is in hundredths of degree Celsius
    pressure = press_comp / 25600.0f;       // Pressure is in Pa, convert to hPa by dividing by 25600
    humidity = hum_comp / 1024.0f;          // Humidity is in thousand
}

void BME280::sample_data(uint8_t address, uint8_t &data){
    //Create variable for data buffer
    uint8_t data_buffer = 0;

    //Read the existing data register
    register_read(address, &data_buffer, 1);

    //write new data
    data_buffer |= data;

    //write data over SPI
    register_write(address, data_buffer);
}

void BME280::pressure_oversample(oversample_e os){
    //Convert the oversampling enum to a uint8_t value
    uint8_t oversample_value = (uint8_t)os;

    //Shift it to correct bit position for pressure field.
    uint8_t shifted_value = oversample_value << PRESS_OVERSAMPLE_SHIFT;

    //Write the updated value to the CTRL_MEAS register
    sample_data(CTRL_MEAS, shifted_value);
}

void BME280::humidity_oversample(oversample_e os){
    //Convert the oversampling enum to a uint8_t value
    uint8_t oversample_value = (uint8_t)os;

    //Shift it to correct bit position for humidity field.
    uint8_t shifted_value = oversample_value << HUM_OVERSAMPLE_SHIFT;

    //Write the updated value to the CTRL_HUM register
    sample_data(CTRL_HUM, shifted_value);

}

void BME280::temperature_oversample(oversample_e os){
    //Convert the oversampling enum to a uint8_t value
    uint8_t oversample_value = (uint8_t)os;

    //Shift it to correct bit position for temperature field.
    uint8_t shifted_value = oversample_value << TEMP_OVERSAMPLE_SHIFT;

    //Write the updated value to the CTRL_MEAS register
    sample_data(CTRL_MEAS, shifted_value);
}

void BME280::set_force_mode(void){
    //write data
    uint8_t data = FORCED_MODE;

    //write the data over SPI
    register_write(CTRL_MEAS, data);
}

void BME280::set_normal_mode(void){
    //write data
    uint8_t data = NORMAL_MODE;

    //write the data over SPI
    register_write(CTRL_MEAS, data);
}

BME280_S32_t BME280::compensate_T_int32(BME280_S32_t adc_T){

}

BME280_U32_t BME280::compensate_P_int64(BME280_S32_t adc_P){

}



