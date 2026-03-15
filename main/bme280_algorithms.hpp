#ifndef BME280_ALGORITHMS_HPP
#define BME280_ALGORITHMS_HPP

#include <cstddef>
#include <cstdint>

namespace bme280 {

struct CalibrationData {
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
};

class Algorithms {
public:
    static int32_t CompensateTemperatureInt32(int32_t adc_temperature,
                                              const CalibrationData &calibration,
                                              int32_t *t_fine_out);

    static uint32_t CompensatePressureInt64(int32_t adc_pressure,
                                            const CalibrationData &calibration,
                                            int32_t t_fine);

    static uint32_t CompensateHumidityInt32(int32_t adc_humidity,
                                            const CalibrationData &calibration,
                                            int32_t t_fine);

    static bool IsTransferSizeValid(std::size_t requested, std::size_t max_transfer);

    static bool CopyPayloadSafely(const uint8_t *rx_buffer,
                                  std::size_t rx_len,
                                  uint8_t *out_buffer,
                                  std::size_t out_len,
                                  std::size_t payload_len);
};

}  // namespace bme280

#endif
