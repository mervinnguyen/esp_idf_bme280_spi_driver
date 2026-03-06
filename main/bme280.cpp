
#include "bme280.hpp"

//ESP-IDF SDK
#include "driver/spi_common.h"
#include "hal/spi_types.h"
#include "soc/soc.h"

// BME280 Constructor: Initializes the BME280 sensor with SPI configuration
// Parameters:
//   devcfg - SPI device interface configuration (clock speed, mode, CS pin, etc.)
//   bus_config - SPI bus configuration (MISO, MOSI, CLK pins)
//   spi_dev - Reference to SPI device handle (will be populated by spi_bus_add_device)
BME280::BME280(const spi_device_interface_config_t &devcfg, const spi_bus_config_t &bus_config, spi_device_handle_t &spi_dev) : devcfg{ devcfg }, buscfg{busconfig}, spi_dev{spi_dev} 
{
    // Initialize SPI bus with the provided configuration on VSPI_HOST
    // DMA channel 1 is used for efficient data transfer
    spi_bus_initialize(VSPI_HOST, &buscfg, 1u);
    
    // Attach BME280 device to the initialized SPI bus
    // This populates the spi_dev handle for future transactions
    spi_bus_add_device(VSPI_HOST, &devcfg, &spi_dev);

    // Read and store BME280 calibration coefficients from non-volatile memory
    // These are required for accurate temperature, pressure, and humidity calculations
    get_calibration_data();
}

void BME280::get_calibration_data(void){
    register_read(ADDR_DIG_T1, (uint8_t*)&(calibration_data.dig_T1), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_T2, (uint8_t*)&(calibration_data.dig_T2), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_T3, (uint8_t*)&(calibration_data.dig_T3), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P1, (uint8_t*)&(calibration_data.dig_P1), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P2, (uint8_t*)&(calibration_data.dig_P2), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P3, (uint8_t*)&(calibration_data.dig_P3), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P4, (uint8_t*)&(calibration_data.dig_P4), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P5, (uint8_t*)&(calibration_data.dig_P5), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P6, (uint8_t*)&(calibration_data.dig_P6), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P7, (uint8_t*)&(calibration_data.dig_P7), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P8, (uint8_t*)&(calibration_data.dig_P8), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_P9, (uint8_t*)&(calibration_data.dig_P9), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_H1, (uint8_t*)&(calibration_data.dig_H1), CAL_SIZE_1_BYTE);
    register_read(ADDR_DIG_H2, (uint8_t*)&(calibration_data.dig_H2), CAL_SIZE_2_BYTE);
    register_read(ADDR_DIG_H3, (uint8_t*)&(calibration_data.dig_H3), CAL_SIZE_1_BYTE);

    register_read(ADDR_DIG_H4, (uint8_t*)&(calibration_data.dig_H4), CAL_SIZE_2_BYTE);
    calibration_data.dig_H4 = ((calibration_data.dig_H4 >> 8u) & 0xF) |
                              ((calibration_data.dig_H4 & 0xFF) << 4u);

    register_read(ADDR_DIG_H5, (uint8_t*)&(calibration_data.dig_H5), CAL_SIZE_2_BYTE);
    calibration_data.dig_H5 = (((calibration_data.dig_H5 & 0xF) << 4u) |
                                (calibration_data.dig_H5 & 0xFF00)) >> 4u;

    register_read(ADDR_DIG_H6, (uint8_t*)&(calibration_data.dig_H6), CAL_SIZE_1_BYTE);
}

void BME280::clear_all_registers(void){
    uint8_t tx_buffer
}