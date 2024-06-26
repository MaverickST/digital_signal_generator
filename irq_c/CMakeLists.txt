cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD "pico_w")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)


project(SignalGenerator)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(signal_irq
	main.c
	functs.c
	dac.c
	keypad_irq.c
	signal_generator_irq.c
)

target_include_directories(signal_irq PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(signal_irq 
	pico_stdlib 
	hardware_timer
	pico_cyw43_arch_none 
	hardware_gpio 
	hardware_pwm 
	hardware_irq 
	hardware_sync)

pico_enable_stdio_uart(signal_irq 0)
pico_enable_stdio_usb(signal_irq 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(signal_irq)

# Memory usage
SET(GCC_EXE_LINKER_FLAGS    "-Wl,--print-memory-usage")

SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_EXE_LINKER_FLAGS}")
