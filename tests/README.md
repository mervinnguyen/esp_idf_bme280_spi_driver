# GoogleTest Host Unit Tests

This directory contains host-side unit tests for logic that is difficult to validate on target hardware, including:

- Compensation formula verification
- Buffer and transfer-size safety checks for memory-safe handling

## Run tests

From the repository root:

```bash
cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

## Notes

- These tests are host-native and do not require flashing an ESP device.
- They intentionally target pure logic from `main/bme280_algorithms.*` so they can run fast in CI.
