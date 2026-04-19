#/**
 * @file spi.cpp
 * @brief SPI driver and task setup for interfacing ESP32 with BME280 sensor using ESP-IDF.
 *
 * This file configures the SPI bus and device, and provides FreeRTOS tasks for reading sensor data
 * from a BME280 environmental sensor (temperature, pressure, humidity) over SPI. The code demonstrates
 * both forced and normal measurement modes, and prints sensor data to the console.
 *
 * @author Mervin Nguyen
 * @date 2026
 */

// Standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

// ESP-IDF SDK
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/clk_tree_defs.h"
#include "esp_log.h"

// Local driver
#include "bme280.hpp"

extern "C" { void app_main(); }

#define PIN_NUM_MISO 19 
#define PIN_NUM_MOSI 23 
#define PIN_NUM_CLK  18
#define PIN_NUM_CS    5

#define SPI_HOST SPI2_HOST

/**
 * @brief SPI bus configuration for ESP32.
 */
spi_bus_config_t buscfg = [] {
    spi_bus_config_t cfg{};
    cfg.miso_io_num = PIN_NUM_MISO;        ///< GPIO for MISO
    cfg.mosi_io_num = PIN_NUM_MOSI;        ///< GPIO for MOSI
    cfg.sclk_io_num = PIN_NUM_CLK;         ///< GPIO for SCLK
    cfg.quadwp_io_num = -1;                ///< Not used (Quad SPI only)
    cfg.quadhd_io_num = -1;                ///< Not used (Quad SPI only)
    cfg.data4_io_num = -1;                 ///< Not used (Quad SPI only)
    cfg.data5_io_num = -1;                 ///< Not used (Quad SPI only)
    cfg.data6_io_num = -1;                 ///< Not used (Quad SPI only)
    cfg.data7_io_num = -1;                 ///< Not used (Quad SPI only)
    cfg.max_transfer_sz = 32;              ///< Max transfer size (bytes)
    cfg.flags = 0;                         ///< No special flags
    cfg.intr_flags = 0;                    ///< No special interrupt flags
    return cfg;
}();


/**
 * @brief SPI device interface configuration for BME280 sensor.
 */
spi_device_interface_config_t devcfg = [] {
    spi_device_interface_config_t dfg{};
    dfg.command_bits    = 0;                          ///< No command phase
    dfg.address_bits    = 0;                          ///< No address phase
    dfg.dummy_bits      = 0;                          ///< No dummy bits
    dfg.mode            = 0;                          ///< SPI Mode 0 (CPOL=0, CPHA=0)
    dfg.clock_source    = SPI_CLK_SRC_DEFAULT;        ///< Default clock source
    dfg.duty_cycle_pos  = 128;                        ///< 50% duty cycle
    dfg.cs_ena_pretrans = 1;                          ///< No CS enable delay before
    dfg.cs_ena_posttrans = 0;                         ///< No CS enable delay after
    dfg.clock_speed_hz  = 1000000;                    ///< 1 MHz SPI clock
    dfg.input_delay_ns  = 0;                          ///< No input delay
    dfg.spics_io_num    = PIN_NUM_CS;                 ///< Chip Select GPIO
    dfg.flags           = 0;                          ///< No special flags
    dfg.queue_size      = 7;                          ///< Transaction queue size
    dfg.pre_cb          = nullptr;                    ///< No pre-transaction callback
    dfg.post_cb         = nullptr;                    ///< No post-transaction callback
    dfg.sample_point    = SPI_SAMPLING_POINT_PHASE_0; ///< Default sampling point
    return dfg;
}();

/**
 * @brief FreeRTOS task for reading BME280 sensor in forced mode.
 *
 * In forced mode, the sensor performs a single measurement when triggered, then returns to sleep.
 * This task demonstrates how to trigger measurements and read data periodically.
 * @param pvParameters Not used.
 */
void task_forced_mode(void *pvParameters) {
    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spi_dev);

    bme.clear_all_registers();
    bme.pressure_oversample(oversample_1x);      ///< Pressure oversampling: 1x
    bme.humidity_oversample(oversample_1x);      ///< Humidity oversampling: 1x
    bme.temperature_oversample(oversample_1x);   ///< Temperature oversampling: 1x

    for (;;) {
        bme.set_force_mode();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        bme.burst_read_data();

        printf("Temperature: %.2f C\n", bme.temperature);
        printf("Pressure: %.2f hPa\n", bme.pressure);
        printf("Humidity: %.2f %%\n", bme.humidity);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before the next measurement cycle
    }
}


/**
 * @brief FreeRTOS task to read and print the BME280 chip ID.
 *
 * This task demonstrates how to communicate with the sensor and verify its presence.
 * @param pvParameters Not used.
 */
void task_read_chip_id(void *pvParameters) {
    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spi_dev);
    uint8_t chip_id = bme.read_chip_id();
    printf("BME280 chip ID: 0x%02X\n", chip_id);
    vTaskDelete(NULL);
}


/**
 * @brief FreeRTOS task for reading BME280 sensor in normal (continuous) mode.
 *
 * In normal mode, the sensor continuously measures and updates data registers.
 * This task configures high-accuracy oversampling and prints sensor data periodically.
 * @param pvParameters Not used.
 */
void task_normal_mode(void *pvParameters) {
    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spi_dev);

    bme.clear_all_registers();
    bme.pressure_oversample(oversample_16x);     ///< Pressure oversampling: 16x
    bme.humidity_oversample(oversample_16x);     ///< Humidity oversampling: 16x
    bme.temperature_oversample(oversample_16x);  ///< Temperature oversampling: 16x
    bme.set_normal_mode();

    for (;;) {
        bme.burst_read_data();
        printf("Temperature: %.2f C\n", bme.temperature);
        printf("Pressure: %.2f hPa\n", bme.pressure);
        printf("Humidity: %.2f %%\n", bme.humidity);

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before the next measurement cycle
    }
}

void app_main(void){
    //create tasks
    xTaskCreate(task_read_chip_id, "task_read_chip_id", 3072, NULL, 6, NULL);
    //xTaskCreate(task_normal_mode, "task_normal_mode", 4096, NULL, 5, NULL);
    xTaskCreate(task_forced_mode, "task_forced_mode", 4096, NULL, 5, NULL);
}
