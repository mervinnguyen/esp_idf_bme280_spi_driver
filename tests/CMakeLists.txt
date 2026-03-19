cmake_minimum_required(VERSION 3.20)
project(bme280_host_tests LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(bme280_algorithms STATIC
  ../main/bme280_algorithms.cpp
)

target_include_directories(bme280_algorithms PUBLIC
  ../main
)

add_executable(bme280_unit_tests
  test_bme280_algorithms.cpp
)

target_link_libraries(bme280_unit_tests
  PRIVATE
  bme280_algorithms
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(bme280_unit_tests)
