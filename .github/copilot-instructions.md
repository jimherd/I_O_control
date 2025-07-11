# Copilot Instructions for Pi_Robot_test (RP2040 Pico Project)

## Project Overview
- This project controls an animatronic robot head using a Raspberry Pi Pico (RP2040) microcontroller.
- Major hardware: servo motors (via PCA9685), neck stepper motor (TMC2208), Neopixel RGB LEDs.
- Firmware is written in C, using FreeRTOS for multitasking and the Pico SDK for hardware abstraction.

## Architecture & Key Components
- Source code: `src/` (main logic, hardware drivers, FreeRTOS tasks)
- Headers: `include/` (interfaces for hardware and system modules)
- FreeRTOS kernel: `lib/FreeRTOS-Kernel/` (local copy, not submodule)
- Build system: CMake (`CMakeLists.txt`), Ninja, Pico SDK, custom toolchain
- Hardware abstraction: Pico SDK, custom drivers for PCA9685, TMC2208, Neopixel
- PIO: Custom PIO program for Neopixel (`src/neopixel.pio`), header generated to `generated/`

## Build & Flash Workflow
- Build: Use VS Code tasks or run Ninja in `build/` (see CMakeLists.txt for details)
- Flash: Use Picotool or OpenOCD via VS Code tasks
- Output binaries: `build/` (e.g., `.elf`, `.uf2`, `.bin`)
- File size tracking: `File_size.txt` updated post-build
- SDK/toolchain paths are set via environment variables and CMake logic

## Developer Patterns & Conventions
- All source files are auto-included via CMake `FILE(GLOB ...)` in `src/`
- FreeRTOS is statically linked; kernel source and port files are specified in CMake
- Hardware drivers are separated by function (e.g., `PCA9685.c`, `Task_servo_control.c`)
- PIO header is generated at build time for Neopixel
- UART/USB serial is disabled by default (`pico_enable_stdio_usb`, `pico_enable_stdio_uart`)
- Use `externs.h` for global extern declarations
- Custom minimal printf implementation (`min_printf.c`)

## Integration Points
- Pico SDK: Path set via environment, imported in CMake
- FreeRTOS: Local kernel, ported for RP2040, imported in CMake
- Hardware: PCA9685 (I2C), TMC2208 (Stepper, UART), Neopixel (PIO)

## Example: Adding a New Hardware Driver
1. Create `src/MyDriver.c` and `include/MyDriver.h`
2. Implement driver logic and task if needed
3. CMake will auto-include new `.c` files in build
4. Add necessary includes to main or relevant task

## Key Files
- `CMakeLists.txt`: Build logic, SDK/toolchain setup, source file collection
- `src/main.c`: Entry point, task creation
- `include/`: All hardware/system headers
- `lib/FreeRTOS-Kernel/`: RTOS source and port
- `src/neopixel.pio`: PIO program for Neopixel

## External Dependencies
- Pico SDK (local, not submodule)
- FreeRTOS (local, not submodule)
- Ninja, CMake, OpenOCD, Picotool, Arm GNU toolchain

# Project general coding standards

## Naming Conventions
- Use snake case
- Use K&R coding style
- Use ALL_CAPS for constants

---
If any section is unclear or missing, please specify what needs improvement or further detail.
