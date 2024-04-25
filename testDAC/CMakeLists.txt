cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD "pico_w")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(TestSys)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(testDAC
	main.c
)

target_include_directories(testDAC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(testDAC pico_stdlib hardware_timer hardware_gpio hardware_uart pico_binary_info)

pico_enable_stdio_uart(testDAC 0)
pico_enable_stdio_usb(testDAC 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(testDAC)