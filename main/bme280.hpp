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
#define ADDR_DIG_P6     (0x98)
#define ADDR_DIG_P7     (0x9A)
#define ADDR_DIG_P8     (0x9C)
#define ADDR_DIG_P9     (0x9F)
#define ADDR_DIG_H1     (0xA1)
#define ADDR_DIG_H2     (0xE1)
#define ADDR_DIG_H3     (0xE3)
#define ADDR_DIG_H4     (0xE4)
#define ADDR_DIG_H5     (0xE5)
#define ADDR_DIG_H6     (0xE7)