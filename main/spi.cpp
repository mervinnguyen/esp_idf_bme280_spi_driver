//Standard libraries
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

//esp-idf SDK
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/clk_tree_defs.h"
#include "esp_log.h"

//local driver
#include "bme280.hpp"

extern "C" { void app_main(); }

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5

#define SPI_HOST SPI2_HOST

// SPI Bus Configuration: Defines the GPIO pins for the SPI bus
spi_bus_config_t buscfg = [] {
    spi_bus_config_t cfg{};
    cfg.miso_io_num = PIN_NUM_MISO;        // GPIO 19: Master In Slave Out (data from BME280)
    cfg.mosi_io_num = PIN_NUM_MOSI;        // GPIO 23: Master Out Slave In (data to BME280)
    cfg.sclk_io_num = PIN_NUM_CLK;         // GPIO 18: SPI Clock
    cfg.quadwp_io_num = -1;                // Quad Write Protect (not used in standard SPI)
    cfg.quadhd_io_num = -1;                // Quad Hold (not used in standard SPI)
    cfg.data4_io_num = -1;                 // Extra data line 4 (disabled - not using Quad SPI)
    cfg.data5_io_num = -1;                 // Extra data line 5 (disabled - not using Quad SPI)
    cfg.data6_io_num = -1;                 // Extra data line 6 (disabled - not using Quad SPI)
    cfg.data7_io_num = -1;                 // Extra data line 7 (disabled - not using Quad SPI)
    cfg.max_transfer_sz = 32;              // Maximum transfer size in bytes (sufficient for register read/write operations)
    cfg.flags = 0;                         // No special flags needed for standard SPI operation
    cfg.intr_flags = 0;                    // No special interrupt flags needed for this application
    return cfg;
}();

// SPI Device Interface Configuration: Defines BME280 device-specific settings
spi_device_interface_config_t devcfg = [] {
    spi_device_interface_config_t dfg{};
    dfg.command_bits    = 0;                          // No command phase (BME280 uses register address as first byte)
    dfg.address_bits    = 0;                          // No address phase (address embedded in data)
    dfg.dummy_bits      = 0;                          // No dummy bits needed for BME280
    dfg.mode            = 0;                          // SPI Mode 0 (CPOL=0, CPHA=0) - correct for BME280
    dfg.clock_source    = SPI_CLK_SRC_DEFAULT;        // Use default clock source
    dfg.duty_cycle_pos  = 128;                        // 50% duty cycle (clock high/low equal time)
    dfg.cs_ena_pretrans = 1;                          // No CS enable delay before transmission
    dfg.cs_ena_posttrans = 0;                         // No CS enable delay after transmission
    dfg.clock_speed_hz  = 1000000;                    // 1 MHz clock speed (within BME280's 10 MHz max)
    dfg.input_delay_ns  = 0;                          // No input delay compensation
    dfg.spics_io_num    = PIN_NUM_CS;                 // GPIO 5: Chip Select pin for BME280
    dfg.flags           = 0;                          // No special flags (standard SPI operation)
    dfg.queue_size      = 7;                          // Allow 7 SPI transactions to be queued
    dfg.pre_cb          = nullptr;                    // No callback before transaction
    dfg.post_cb         = nullptr;                    // No callback after transaction
    dfg.sample_point    = SPI_SAMPLING_POINT_PHASE_0; // Default sampling point
    return dfg;
}();

void task_forced_mode(void *pvParameters){
    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spi_dev);

    // Reset all BME280 registers to their default values
    bme.clear_all_registers();
    
    // Configure oversampling settings (1x = no oversampling, faster but less accurate)
    bme.pressure_oversample(oversample_1x);      // Set pressure oversampling to 1x
    bme.humidity_oversample(oversample_1x);      // Set humidity oversampling to 1x
    bme.temperature_oversample(oversample_1x);   // Set temperature oversampling to 1x
    // Note: Higher oversampling (2x, 4x, 8x, 16x) provides more accurate readings
    // but increases measurement time and power consumption

    for (;;){

        bme.set_force_mode(); // Trigger a single measurement in forced mode    
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second before the next measurement
        bme.burst_read_data(); // Read all sensor data (temperature, pressure, humidity) in one burst read

        printf("Temperature: %.2f C\n", bme.temperature); // Print temperature in Celsius
        printf("Pressure: %.2f hPa\n", bme.pressure); // Print pressure in hPa
        printf("Humidity: %.2f %%\n", bme.humidity); // Print humidity in percentage

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before the next measurement cycle
    }
}

void task_normal_mode(void *pvParameters){

    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spi_dev);
    const uint8_t chip_id = bme.read_chip_id();
    printf("BME280 chip ID: 0x%02X\n", chip_id);

    bme.clear_all_registers(); // Reset all BME280 registers to their default values
    bme.pressure_oversample(oversample_16x);     // Set pressure oversampling to 16x for higher accuracy
    bme.humidity_oversample(oversample_16x);     // Set humidity oversampling to 16x for higher accuracy
    bme.temperature_oversample(oversample_16x);  // Set temperature oversampling to 16x for higher accuracy
    bme.set_normal_mode(); // Set BME280 to normal mode for continuous measurements

    for (;;){
        bme.burst_read_data(); // Read all sensor data (temperature, pressure, humidity) in one burst read
        printf("Temperature: %.2f C\n", bme.temperature); // Print temperature in Celsius
        printf("Pressure: %.2f hPa\n", bme.pressure); // Print pressure in hPa
        printf("Humidity: %.2f %%\n", bme.humidity); // Print humidity in percentage

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before the next measurement cycle
    }
}

void app_main(void){
    //create task
    xTaskCreate(task_normal_mode, "task_normal_mode", 4096, NULL, 5, NULL);

    
}
