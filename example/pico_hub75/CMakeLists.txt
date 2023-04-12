cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_hub75 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")


add_executable(pico_hub75
        src/pico_hub75.c
        src/hub75_driver.c
        src/usb_descriptors.c
        ../../lib/c/src/uart_over_midi.c
        )

target_compile_definitions(pico_hub75 PRIVATE
	PICO_DEFAULT_UART_TX_PIN=28
	PICO_DEFAULT_UART_RX_PIN=29
        )

pico_generate_pio_header(pico_hub75
        ${CMAKE_CURRENT_LIST_DIR}/src/hub75.pio)

target_include_directories(pico_hub75 PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../lib/c/include
        )

target_link_libraries(pico_hub75 PRIVATE
        pico_stdlib
        hardware_pio
        tinyusb_device
        tinyusb_board)

pico_add_extra_outputs(pico_hub75)
