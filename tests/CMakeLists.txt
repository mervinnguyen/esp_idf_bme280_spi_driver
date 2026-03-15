test: add host-side GoogleTest suite for BME280 logic and safety checks

Set up a standalone host test target using GoogleTest to validate driver logic
that is difficult to verify through on-device hardware testing alone.

What this commit includes:
- Adds a dedicated host test build configuration with test discovery enabled.
- Introduces a pure-logic algorithms module for deterministic unit testing of:
  - temperature compensation math
  - pressure compensation math
  - humidity compensation math and clamping behavior
- Adds memory-safety focused helpers and tests for:
  - transfer size boundary validation
  - safe payload copy behavior
  - rejection of oversized copy requests
- Adds test documentation describing build and run commands.
- Adds ignore rules for generated host test build artifacts.

Why this approach:
- Keeps hardware-dependent code paths separate from host unit tests.
- Enables fast, repeatable CI-friendly validation of core math/safety logic.
- Reduces risk of regressions in compensation formulas and buffer handling.

Validation:
- Host test target configures, builds, and runs successfully.
- All added unit tests pass.test: add host-side GoogleTest suite for BME280 logic and safety checks

Set up a standalone host test target using GoogleTest to validate driver logic
that is difficult to verify through on-device hardware testing alone.

What this commit includes:
- Adds a dedicated host test build configuration with test discovery enabled.
- Introduces a pure-logic algorithms module for deterministic unit testing of:
  - temperature compensation math
  - pressure compensation math
  - humidity compensation math and clamping behavior
- Adds memory-safety focused helpers and tests for:
  - transfer size boundary validation
  - safe payload copy behavior
  - rejection of oversized copy requests
- Adds test documentation describing build and run commands.
- Adds ignore rules for generated host test build artifacts.

Why this approach:
- Keeps hardware-dependent code paths separate from host unit tests.
- Enables fast, repeatable CI-friendly validation of core math/safety logic.
- Reduces risk of regressions in compensation formulas and buffer handling.

Validation:
- Host test target configures, builds, and runs successfully.
- All added unit tests pass.cmake_minimum_required(VERSION 3.20)
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
