idf_component_register(
    SRCS "spi.cpp" "bme280.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_spi freertos
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++14)