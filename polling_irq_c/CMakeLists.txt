cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD "pico_w")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(SignalGenerator)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(signal_polling_irq
	main.c
	functs.c
	dac.c
	keypad.c
	signal_generator.c
)

target_include_directories(signal_polling_irq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(signal_polling_irq 
	pico_stdlib 
	hardware_timer
	pico_cyw43_arch_none 
	hardware_gpio 
	hardware_pwm 
	hardware_irq 
	hardware_sync)

pico_enable_stdio_uart(signal_polling_irq 0)
pico_enable_stdio_usb(signal_polling_irq 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(signal_polling_irq)
