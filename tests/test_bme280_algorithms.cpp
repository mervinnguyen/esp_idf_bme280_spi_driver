#include <array>

#include "gtest/gtest.h"

#include "bme280_algorithms.hpp"

namespace {

bme280::CalibrationData MakeDatasheetCalibration() {
    // Bosch example calibration values used in many BMP/BME280 references.
    return bme280::CalibrationData{
        .dig_T1 = 27504,
        .dig_T2 = 26435,
        .dig_T3 = -1000,
        .dig_P1 = 36477,
        .dig_P2 = -10685,
        .dig_P3 = 3024,
        .dig_P4 = 2855,
        .dig_P5 = 140,
        .dig_P6 = -7,
        .dig_P7 = 15500,
        .dig_P8 = -14600,
        .dig_P9 = 6000,
        .dig_H1 = 75,
        .dig_H2 = 362,
        .dig_H3 = 0,
        .dig_H4 = 315,
        .dig_H5 = 50,
        .dig_H6 = 30,
    };
}

TEST(CompensationMathTest, TemperatureMatchesReferenceExample) {
    const auto calibration = MakeDatasheetCalibration();

    int32_t t_fine = 0;
    const int32_t temp_c_x100 =
        bme280::Algorithms::CompensateTemperatureInt32(519888, calibration, &t_fine);

    EXPECT_EQ(temp_c_x100, 2508);
    EXPECT_GT(t_fine, 0);
}

TEST(CompensationMathTest, PressureMatchesReferenceExample) {
    const auto calibration = MakeDatasheetCalibration();

    int32_t t_fine = 0;
    (void)bme280::Algorithms::CompensateTemperatureInt32(519888, calibration, &t_fine);

    // Expected result in Q24.8 format from Bosch integer algorithm examples.
    const uint32_t pressure_q24_8 =
        bme280::Algorithms::CompensatePressureInt64(415148, calibration, t_fine);

    EXPECT_EQ(pressure_q24_8, 25767233U);
}

TEST(CompensationMathTest, HumidityOutputIsClampedToRange) {
    const auto calibration = MakeDatasheetCalibration();

    int32_t t_fine = 0;
    (void)bme280::Algorithms::CompensateTemperatureInt32(519888, calibration, &t_fine);

    const uint32_t humidity =
        bme280::Algorithms::CompensateHumidityInt32(367, calibration, t_fine);

    // Output is in Q22.10 (0..102400 corresponds to 0..100%RH).
    EXPECT_LE(humidity, 102400U);
}

TEST(BufferSafetyTest, TransferSizeValidationWorks) {
    EXPECT_TRUE(bme280::Algorithms::IsTransferSizeValid(1, 32));
    EXPECT_TRUE(bme280::Algorithms::IsTransferSizeValid(32, 32));
    EXPECT_FALSE(bme280::Algorithms::IsTransferSizeValid(0, 32));
    EXPECT_FALSE(bme280::Algorithms::IsTransferSizeValid(33, 32));
}

TEST(BufferSafetyTest, CopyPayloadSafelyCopiesWithoutOverflow) {
    const std::array<uint8_t, 6> rx = {0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    std::array<uint8_t, 4> out = {0};

    const bool copied =
        bme280::Algorithms::CopyPayloadSafely(rx.data(), rx.size(), out.data(), out.size(), out.size());

    ASSERT_TRUE(copied);
    EXPECT_EQ(out[0], 0xAA);
    EXPECT_EQ(out[1], 0xBB);
    EXPECT_EQ(out[2], 0xCC);
    EXPECT_EQ(out[3], 0xDD);
}

TEST(BufferSafetyTest, CopyPayloadSafelyRejectsOversizedRequest) {
    const std::array<uint8_t, 4> rx = {0x00, 0x10, 0x20, 0x30};
    std::array<uint8_t, 2> out = {0};

    const bool copied =
        bme280::Algorithms::CopyPayloadSafely(rx.data(), rx.size(), out.data(), out.size(), 3);

    EXPECT_FALSE(copied);
}

}  // namespace
