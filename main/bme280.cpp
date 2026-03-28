#include "bme280.hpp"

#include "driver/spi_common.h"
#include "hal/spi_types.h"
#include "soc/soc.h"
#include "freertos/FreeRTOS.h"


BME280::BME280(const spi_device_interface_config_t &devcfg, const spi_bus_config_t &bus_config, spi_device_handle_t &spi_dev)
    : dev_cfg{devcfg}, bus_cfg{bus_config}, spi_dev{spi_dev}
{
    spi_bus_initialize(SPI3_HOST, &bus_config, 1u);
    spi_bus_add_device(SPI3_HOST, &devcfg, &spi_dev);
    get_calibration_data();
}

/**
 * @brief Retrieve calibration coefficients from BME280 non-volatile memory.
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
    uint8_t tx_buffer[6] = {
        CTRL_MEAS & REG_WRITE_ONLY, 0x00,
        CONFIG & REG_WRITE_ONLY,    0x00,
        CTRL_HUM & REG_WRITE_ONLY,  0x00,
    };
    spi_transaction_t trans{};
    trans.flags = 0;
    trans.cmd = 0;
    trans.addr = 0;
    trans.length = 8 * 6;
    trans.rxlength = 0;
    trans.user = nullptr;
    trans.tx_buffer = tx_buffer;
    trans.rx_buffer = nullptr;
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);
    spi_device_transmit(spi_dev, &trans);
    spi_device_release_bus(spi_dev);
}

void BME280::register_write(uint8_t address, uint8_t data){
    uint8_t tx_buffer[2];
    tx_buffer[0] = address | REG_READ_ONLY;
    tx_buffer[1] = data;
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.length = 16;
    trans.tx_buffer = tx_buffer;
    trans.rx_buffer = 0;
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);
    spi_device_transmit(spi_dev, &trans);
    spi_device_release_bus(spi_dev);
}

void BME280::register_read(const uint8_t address, uint8_t *buffer, const uint8_t size){
    uint8_t tx_buffer[MAX_TX_BUFFER_SIZE] = {0};
    uint8_t rx_buffer[MAX_RX_BUFFER_SIZE] = {0};
    tx_buffer[0] = address | REG_READ_ONLY;
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));

    //Total bits to send and recieve (address + data)
    trans.length = 8 * (1 + size); // 1 byte for address + size bytes for data 
    trans.rxlength = 8 * (size + 1); // Only the data bytes are expected in response

    //Assign buffers
    trans.tx_buffer = tx_buffer;
    trans.rx_buffer = rx_buffer;
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);
    spi_device_transmit(spi_dev, &trans);
    spi_device_release_bus(spi_dev);
    memcpy(buffer, &rx_buffer[1], size);
}

void BME280::burst_read_data(void){
    uint8_t tx_buffer[9] = {0};
    tx_buffer[0] = PRESS_MSB | REG_READ_ONLY;
    uint8_t rx_buffer[9] = {0};
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.length = 8 * 9;
    trans.rxlength = 8 * 9;
    trans.tx_buffer = tx_buffer;
    trans.rx_buffer = rx_buffer;
    spi_device_acquire_bus(spi_dev, portMAX_DELAY);
    spi_device_transmit(spi_dev, &trans);
    spi_device_release_bus(spi_dev);
    uint32_t press_msb = rx_buffer[1];
    uint32_t press_lsb = rx_buffer[2];
    uint32_t press_xlsb = rx_buffer[3];
    uint32_t raw_pressure = (press_msb << 16) | (press_lsb << 8) | (press_xlsb);

    raw_pressure >>= 4; // Pressure is 20 bits, so we need to shift right by 4 to align it

    //Combine temperature bytes
    uint32_t temp_msb = rx_buffer[4];
    uint32_t temp_lsb = rx_buffer[5];
    uint32_t temp_xlsb = rx_buffer[6];
    uint32_t raw_humidity = (HUM_MSB << 8) | HUM_LSB;
    uint32_t raw_temperature = (temp_msb << 16) | (temp_lsb << 8) | (temp_xlsb);
    raw_temperature >>= 4;
    int32_t temp_comp = compensate_T_int32(raw_temperature);
    int32_t press_comp = compensate_P_int64(raw_pressure);
    int32_t hum_comp = compensate_H_int32(raw_humidity);

    //Convert to floats
    temperature = temp_comp / 100.0f;        // Temperature is in hundredths of degree Celsius
    pressure = press_comp / 25600.0f;       // Pressure is in Pa, convert to hPa by dividing by 25600
    humidity = hum_comp / 1024.0f;          // Humidity is in thousands
}

void BME280::sample_data(const uint8_t address, const uint8_t &data){
    uint8_t data_buffer = 0;
    register_read(address, &data_buffer, 1);
    data_buffer |= data;
    register_write(address, data_buffer);
}

void BME280::pressure_oversample(oversample_e os){
    uint8_t oversample_value = (uint8_t)os;
    uint8_t shifted_value = oversample_value << PRESS_OVERSAMPLE_SHIFT;
    sample_data(CTRL_MEAS, shifted_value);
}

void BME280::humidity_oversample(oversample_e os){
    uint8_t oversample_value = (uint8_t)os;
    uint8_t shifted_value = oversample_value << HUM_OVERSAMPLE_SHIFT;
    sample_data(CTRL_HUM, shifted_value);
}

void BME280::temperature_oversample(oversample_e os){
    uint8_t oversample_value = (uint8_t)os;
    uint8_t shifted_value = oversample_value << TEMP_OVERSAMPLE_SHIFT;
    sample_data(CTRL_MEAS, shifted_value);
}

void BME280::set_force_mode(void){
    uint8_t data = FORCED_MODE;
    register_write(CTRL_MEAS, data);
}

void BME280::set_normal_mode(void){
    uint8_t data = NORMAL_MODE;
    register_write(CTRL_MEAS, data);
}

uint8_t BME280::read_chip_id(void){
    uint8_t id = 0;
    register_read(CHIP_ID_REG, &id, 1);
    return id;
}

BME280_S32_t BME280::compensate_T_int32(BME280_S32_t adc_T){
    BME280_S32_t var1, var2, temperature = 0;

    //Compensation calculation from datasheet
    //Shift raw ADC temperature to the right by 3 bits
    BME280_S32_t shifted_adc_1 = adc_T >> 3;

    //Shift calibration T1 left by 1 bit
    BME280_S32_t shifted_T1 = ((BME280_S32_t)calibration_data.dig_T1) << 1;
    BME280_S32_t diff1 = shifted_adc_1 - shifted_T1;
    BME280_S32_t mult1 = diff1 * ((BME280_S32_t)calibration_data.dig_T2);

    var1 = mult1 >> 11;

    //Second compensation calculation

    //Shift ADC temperature right by 4 bits
    BME280_S32_t shifted_adc_2 = adc_T >> 4;
    BME280_S32_t diff2 = shifted_adc_2 - ((BME280_S32_t)calibration_data.dig_T1);

    BME280_S32_t squared = diff2 * diff2;
    BME280_S32_t shifted_sqr = squared >> 12;
    BME280_S32_t mult2 = shifted_sqr * ((BME280_S32_t)calibration_data.dig_T3);

    //Final shift 
    var2 = mult2 >> 14;
    t_fine = var1 + var2;

    //Final temp calculations
    BME280_S32_t temp_scaled = (t_fine * 5) + 128;
    temperature = temp_scaled >> 8;
    return temperature;
}

BME280_U32_t BME280::compensate_P_int64(BME280_S32_t adc_P){
    //Use 64-bit variables for precision (from datasheet)
    BME280_S64_t var1, var2, pressure = 0;

    //Start with temperature compensaiton value
    var1 = (BME280_S64_t)t_fine - 128000;
    BME280_S64_t var1_squared = var1 * var1;
    var2 = var1_squared * (BME280_S64_t)calibration_data.dig_P6;
    var2 += (var1 * (BME280_S64_t)calibration_data.dig_P5) << 17;
    var2 += ((BME280_S64_t)calibration_data.dig_P4) << 35;

    //More compensation math
    BME280_S64_t temp1 = (var1_squared * (BME280_S64_t)calibration_data.dig_P3) >> 8;
    BME280_S64_t temp2 = (var1 * (BME280_S64_t)calibration_data.dig_P2) << 12;
    var1 = temp1 + temp2;
    BME280_S64_t scale = ((BME280_S64_t)1 << 47) + var1;
    scale = (scale * (BME280_S64_t)calibration_data.dig_P1) >> 33;
    var1 = scale;

    if (var1 == 0) {
        return 0;
    }
    pressure = 1048576 - adc_P;
    pressure = ((pressure << 31) - var2) * 3125 / var1;

    //Final fine adjustments
    BME280_S64_t pressure_shift = pressure >> 13;
    BME280_S64_t adjust1 = ((BME280_S64_t)calibration_data.dig_P9 * (pressure_shift * pressure_shift)) >> 25;
    BME280_S64_t adjust2 = ((BME280_S64_t)calibration_data.dig_P8 * pressure_shift) >> 19;
    pressure = pressure + adjust1 + adjust2;
    pressure = (pressure >> 8) + ((BME280_S64_t)calibration_data.dig_P7 << 4);
    return (BME280_U32_t)pressure;
}

BME280_U32_t BME280::compensate_H_int32(BME280_S32_t adc_H){
    BME280_S32_t humidity = 0;
    BME280_S32_t temp_comp = t_fine - 76800;
    BME280_S32_t part1 = adc_H << 14;
    BME280_S32_t part2 = ((BME280_S32_t)calibration_data.dig_H4) << 20;
    BME280_S32_t part3 = calibration_data.dig_H5 * temp_comp;
    BME280_S32_t intermediate = ((part1 + part2 + part3) + 16384) >> 15;

    //More compensation math
    BME280_S32_t temp1 = (temp_comp * calibration_data.dig_H6) >> 10;
    BME280_S32_t temp2 = ((temp_comp * calibration_data.dig_H3) >> 11) + 32768;
    BME280_S32_t temp3 = ((temp1 * temp2) >> 10) + 2097152;
    BME280_S32_t temp4 = (temp3 * calibration_data.dig_H2 + 8192) >> 14;
    humidity = intermediate * temp4;
    BME280_S32_t correction = (((humidity >> 15) * (humidity >> 15)) >> 7) * calibration_data.dig_H1 >> 4;
    humidity = humidity - correction;
    if (humidity < 0){
        humidity = 0;
    }
    if (humidity > 419430400){
        humidity = 419430400;
    }

    //Final scale
    return (BME280_U32_t)(humidity >> 12);
}

