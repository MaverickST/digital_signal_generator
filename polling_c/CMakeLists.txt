cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD "pico_w")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(AccessSys)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(access
	main.c
	keypad_polling.c
	time_base.c
	dac.c
	signal_generator.c
)

target_include_directories(access PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(access pico_stdlib pico_cyw43_arch_none hardware_timer hardware_gpio)

pico_enable_stdio_uart(access 0)
pico_enable_stdio_usb(access 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(access)