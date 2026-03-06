//Standard libraries
#include <cstdio.h>
#include <cstdlib>
#include <cstring>
#include <cstdint>

//esp-idf SDK
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/clk_tree_defs.h"

#include "bme280.h"

extern "C" { void app_main(); }

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5

// SPI Bus Configuration: Defines the GPIO pins for the SPI bus
spi_bus_config_t buscfg = {
    .miso_io_num = PIN_NUM_MISO,        // GPIO 19: Master In Slave Out (data from BME280)
    .mosi_io_num = PIN_NUM_MOSI,        // GPIO 23: Master Out Slave In (data to BME280)
    .sclk_io_num = PIN_NUM_CLK,         // GPIO 18: SPI Clock
    .quadwp_io_num = -1,                // Quad Write Protect (not used in standard SPI)
    .quadhd_io_num = -1,                // Quad Hold (not used in standard SPI)
    .data4_io_num = -1,                 // Extra data line 4 (disabled - not using Quad SPI)
    .data5_io_num = -1,                 // Extra data line 5 (disabled - not using Quad SPI)
    .data6_io_num = -1,                 // Extra data line 6 (disabled - not using Quad SPI)
    .data7_io_num = -1,                 // Extra data line 7 (disabled - not using Quad SPI)
};

// SPI Device Interface Configuration: Defines BME280 device-specific settings
spi_device_interface_config_t devcfg = {
    .command_bits = 0u,                     // No command phase (BME280 uses register address as first byte)
    .address_bits = 0u,                     // No address phase (address embedded in data)
    .dummy_bits = 0u,                       // No dummy bits needed for BME280
    .mode = 0,                              // SPI Mode 0 (CPOL=0, CPHA=0) - correct for BME280
    .clock_source = SPI_CLK_SRC_DEFAULT,    // Use default clock source
    .duty_cycle_pos = 128,                  // 50% duty cycle (clock high/low equal time)
    .cs_ena_pretrans = 0,                   // No CS enable delay before transmission
    .cs_ena_posttrans = 0,                  // No CS enable delay after transmission
    .clock_speed_hz = 10 * 1000 * 1000,     // 10 MHz clock speed (within BME280's 10 MHz max)
    .input_delay_ns = 0,                    // No input delay compensation
    .spics_io_num = PIN_NUM_CS,             // GPIO 5: Chip Select pin for BME280
    .flags = 0,                             // No special flags (standard SPI operation)
    .queue_size = 3,                        // Allow 3 SPI transactions to be queued
    .pre_cb = 0,                            // No callback before transaction
    .post_cb = 0,                           // No callback after transaction
};

void task_forced_mode(void *pvParameters){
    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spidev);

    // Reset all BME280 registers to their default values
    bme.clear_all_register();
    
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

        printf("Temperature: %.2f °C\n", bme.temperature()); // Print temperature in Celsius
        printf("Pressure: %.2f hPa\n", bme.pressure() / 100.0); // Print pressure in hPa (convert from Pa)
        printf("Humidity: %.2f %%\n", bme.humidity()); // Print humidity in percentage

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before the next measurement cycle
    }
}

void task_normal_mode(void *pvParameters){

    spi_device_handle_t spi_dev;
    BME280 bme(devcfg, buscfg, spi_dev);

    bme.clear_all_register(); // Reset all BME280 registers to their default values
    bme.pressure_oversample(oversample_16x);     // Set pressure oversampling to 16x for higher accuracy
    bme.humidity_oversample(oversample_16x);     // Set humidity oversampling to 16x for higher accuracy
    bme.temperature_oversample(oversample_16x);  // Set temperature oversampling to 16x for higher accuracy
    bme.set_normal_mode(); // Set BME280 to normal mode for continuous measurements

    for (;;){
        bme.burst_read_data(); // Read all sensor data (temperature, pressure, humidity) in one burst read
        printf("Temperature: %.2f °C\n", bme.temperature()); // Print temperature in Celsius
        printf("Pressure: %.2f hPa\n", bme.pressure() / 100.0); // Print pressure in hPa (convert from Pa)
        printf("Humidity: %.2f %%\n", bme.humidity()); // Print humidity in percentage

        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for 5 seconds before the next measurement cycle
    }
}

void app_main(void){
    //create task
    xTaskCreate(task_normal_mode, "task_normal_mode", 4096, NULL, 5, NULL);

    
}
