# ==============================================================================
# Battery Management System Firmware — CMakeLists.txt
# SubjuGator 9 — Issue #46
# ==============================================================================
# Hardware:   RP2350 (Raspberry Pi Pico 2)
# Battery IC: TI BQ76920 (I2C)
# ==============================================================================

cmake_minimum_required(VERSION 3.13)

# ---------- SDK Bootstrap ----------
# pico_sdk_import.cmake must be in the same directory as this file.
# It locates the Pico SDK via the PICO_SDK_PATH environment variable.
# This MUST come before the project() call.
include(pico_sdk_import.cmake)

# ---------- Project Declaration ----------
project(battery_management_system C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Abort on any compiler warning during development.
# Comment this out for release builds if desired.
# add_compile_options(-Wall -Wextra -Werror)

# ---------- Initialize the Pico SDK ----------
# This MUST come after the project() call.
pico_sdk_init()

# ---------- Firmware Executable ----------
add_executable(bms_firmware
    src/main.c
    src/bq76920.c
    src/warning_system.c
)

# ---------- Include Paths ----------
# All headers (.h files) live alongside their .c files in src/
target_include_directories(bms_firmware PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---------- Pico SDK Libraries ----------
target_link_libraries(bms_firmware
    pico_stdlib          # GPIO, timers, UART, sleep functions
    hardware_i2c         # I2C peripheral driver (for BQ76920 communication)
    hardware_gpio        # Direct GPIO control (LED, buzzer, MOSFET)
)

# ---------- Output Formats ----------
# Generates .uf2, .hex, .bin, .dis, .elf alongside the main executable.
# The .uf2 is what you drag-and-drop onto the Pico in BOOTSEL mode.
pico_add_extra_outputs(bms_firmware)

# ---------- USB/UART stdio Routing ----------
# Enable USB serial for printf-based debug output and data logging.
# Disable UART to free those pins for other use.
# Flip these values if you prefer UART debug output instead.
pico_enable_stdio_usb(bms_firmware 1)
pico_enable_stdio_uart(bms_firmware 0)
