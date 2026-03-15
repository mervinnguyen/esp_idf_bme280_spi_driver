#include "bme280_algorithms.hpp"

#include <cstring>

namespace bme280 {

int32_t Algorithms::CompensateTemperatureInt32(int32_t adc_temperature,
                                               const CalibrationData &calibration,
                                               int32_t *t_fine_out) {
    const int32_t var1 = ((((adc_temperature >> 3) -
                            (static_cast<int32_t>(calibration.dig_T1) << 1))) *
                          static_cast<int32_t>(calibration.dig_T2)) >>
                         11;

    const int32_t var2 = (((((adc_temperature >> 4) -
                             static_cast<int32_t>(calibration.dig_T1)) *
                            ((adc_temperature >> 4) -
                             static_cast<int32_t>(calibration.dig_T1))) >>
                           12) *
                          static_cast<int32_t>(calibration.dig_T3)) >>
                         14;

    const int32_t t_fine = var1 + var2;
    if (t_fine_out != nullptr) {
        *t_fine_out = t_fine;
    }

    return (t_fine * 5 + 128) >> 8;
}

uint32_t Algorithms::CompensatePressureInt64(int32_t adc_pressure,
                                             const CalibrationData &calibration,
                                             int32_t t_fine) {
    int64_t var1 = static_cast<int64_t>(t_fine) - 128000;
    int64_t var2 = var1 * var1 * static_cast<int64_t>(calibration.dig_P6);
    var2 += (var1 * static_cast<int64_t>(calibration.dig_P5)) << 17;
    var2 += static_cast<int64_t>(calibration.dig_P4) << 35;

    var1 = ((var1 * var1 * static_cast<int64_t>(calibration.dig_P3)) >> 8) +
           ((var1 * static_cast<int64_t>(calibration.dig_P2)) << 12);

    var1 = ((((static_cast<int64_t>(1) << 47) + var1)) *
            static_cast<int64_t>(calibration.dig_P1)) >>
           33;

    if (var1 == 0) {
        return 0;
    }

    int64_t pressure = 1048576 - adc_pressure;
    pressure = (((pressure << 31) - var2) * 3125) / var1;
    var1 = (static_cast<int64_t>(calibration.dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
    var2 = (static_cast<int64_t>(calibration.dig_P8) * pressure) >> 19;

    pressure = ((pressure + var1 + var2) >> 8) +
               (static_cast<int64_t>(calibration.dig_P7) << 4);

    return static_cast<uint32_t>(pressure);
}

uint32_t Algorithms::CompensateHumidityInt32(int32_t adc_humidity,
                                             const CalibrationData &calibration,
                                             int32_t t_fine) {
    int32_t var = t_fine - 76800;

    var = (((((adc_humidity << 14) -
              (static_cast<int32_t>(calibration.dig_H4) << 20) -
              (static_cast<int32_t>(calibration.dig_H5) * var)) +
             16384) >>
            15) *
           (((((((var * static_cast<int32_t>(calibration.dig_H6)) >> 10) *
                (((var * static_cast<int32_t>(calibration.dig_H3)) >> 11) + 32768)) >>
               10) +
              2097152) *
                 static_cast<int32_t>(calibration.dig_H2) +
             8192) >>
            14));

    var = var - (((((var >> 15) * (var >> 15)) >> 7) *
                  static_cast<int32_t>(calibration.dig_H1)) >>
                 4);

    if (var < 0) {
        var = 0;
    }
    if (var > 419430400) {
        var = 419430400;
    }

    return static_cast<uint32_t>(var >> 12);
}

bool Algorithms::IsTransferSizeValid(std::size_t requested, std::size_t max_transfer) {
    return requested > 0 && requested <= max_transfer;
}

bool Algorithms::CopyPayloadSafely(const uint8_t *rx_buffer,
                                   std::size_t rx_len,
                                   uint8_t *out_buffer,
                                   std::size_t out_len,
                                   std::size_t payload_len) {
    if (rx_buffer == nullptr || out_buffer == nullptr) {
        return false;
    }

    if (payload_len > out_len) {
        return false;
    }

    if (rx_len < payload_len + 1) {
        return false;
    }

    std::memcpy(out_buffer, rx_buffer + 1, payload_len);
    return true;
}

}  // namespace bme280
