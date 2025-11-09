# GPIO Test Example

Simple hardware validation test for testing GPIO functionality.

## What it does
- Tests single GPIO reads with LED feedback
- GPIO 18: Enable output
- GPIO 19: Read input
- GPIO 25: LED indicator

## Building
```bash
cd examples/gpio-test
cmake -B build -G Ninja
ninja -C build
```
