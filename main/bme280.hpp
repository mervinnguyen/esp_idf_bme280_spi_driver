#ifndef BME280_HPP
#define BME280_HPP

//Standard libraries
#include <cstdint>
#include <cstring>
#include <cstddef>

//include ESP-IDF SDK
#include "driver/spi_master.h"

#define MAX_RX_BUFFER_SIZE 32
#define MAX_TX_BUFFER_SIZE 32

#define HUM_LSB 0xFE
#define HUM_MSB 0xFD
#define HUM_XLSB 0xFC
#define TEMP_LSB 0xFB
#define TEMP_MSB 0xFA
#define TEMP_XLSB 0xF9
#define PRESS_LSB 0xF8
#define PRESS_MSB 0xF7
#define CONFIG 0xF5
#define CTRL_MEAS 0xF4
#define REG_STATUS 0xF3
#define CTRL_HUM 0xF2

#define ADDR_DIG_T1 0x88
#define ADDR_DIG_T2 0x8A
#define ADDR_DIG_T3 0x8C
#define ADDR_DIG_P1 0x8E
#define ADDR_DIG_P2 0x90
#define ADDR_DIG_P3 0x92
#define ADDR_DIG_P4 0x94
#define ADDR_DIG_P5 0x96
#define ADDR_DIG_P6 0x98
#define ADDR_DIG_P7 0x9A
#define ADDR_DIG_P8 0x9C
#define ADDR_DIG_P9 0x9F
#define ADDR_DIG_H1 0xA1
#define ADDR_DIG_H2 0xE1
#define ADDR_DIG_H3 0xE3
#define ADDR_DIG_H4 0xE4
#define ADDR_DIG_H5 0xE5
#define ADDR_DIG_H6 0xE7

#define CAL_SIZE_1_BYTE 1
#define CAL_SIZE_2_BYTE 2

#define HUM_OVERSAMPLE_SHIFT 0
#define PRESS_OVERSAMPLE_SHIFT 2
#define TEMP_OVERSAMPLE_SHIFT 5

#define REG_READ_ONLY 0x80
#define REG_WRITE_ONLY ~0x80

#define FORCED_MODE 0x01
#define NORMAL_MODE 0x03
#define SLEEP_MODE 0x00

typedef long signed int BME280_S32_t;
typedef long unsigned int BME280_U32_t;
typedef long long signed int BME280_S64_t;

typedef struct
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calib_data_s;

typedef enum
{
    oversample_skip = 0,
    oversample_1x,
    oversample_2x,
    oversample_4x,
    oversample_8x,
    oversample_16x,
} oversample_e;

class BME280
{
    public:
        BME280(const spi_device_interface_config_t &dev_cfg, const spi_bus_config_t &bus_cfg, spi_device_handle_t &spi_dev);

        void clear_all_registers(void);
		void burst_read_data(void);
		void pressure_oversample(oversample_e os);
		void humidity_oversample(oversample_e os);
		void temperature_oversample(oversample_e os);
		void set_force_mode(void);
		void set_normal_mode(void);

        float temperature;
        float pressure;
        float humidity;

    private:
        void sample_data(const uint8_t address, const uint8_t &data);
		void register_read(const uint8_t address, uint8_t *buffer, const uint8_t size);
		void register_write(const uint8_t address, const uint8_t data);
		void get_calibration_data(void);

        BME280_S32_t compensate_T_int32(BME280_S32_t adc_T);
		BME280_U32_t compensate_P_int64(BME280_S32_t adc_P);
		BME280_U32_t compensate_H_int32(BME280_S32_t adc_H);

        const spi_device_interface_config_t &dev_cfg;
        const spi_bus_config_t &bus_cfg;
        spi_device_handle_t &spi_dev;

        bme280_calib_data_s calibration_data;
        BME280_S32_t t_fine;
};

#endif