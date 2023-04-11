cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_uom C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

add_executable(pico_uom
        src/pico_uom.c
        src/usb_descriptors.c
        ../../lib/c/src/uart_over_midi.c
        )

target_include_directories(pico_uom PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/../../lib/c/include
        )

target_link_libraries(pico_uom PRIVATE
    pico_stdlib
    tinyusb_device
    tinyusb_board)
pico_add_extra_outputs(pico_uom)
