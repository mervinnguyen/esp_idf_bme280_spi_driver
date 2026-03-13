idf_component_register(
    SRCS "spi.cpp" "bme280.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_spi freertos
)