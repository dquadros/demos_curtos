cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(gpiobug C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(
    -Wall
    -O3
)

add_executable(gpiobug
    gpiobug.c
    )

# pull in common dependencies
target_link_libraries(gpiobug pico_stdlib hardware_gpio)

# Output via USB
pico_enable_stdio_usb(gpiobug 1)
pico_enable_stdio_uart(gpiobug 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(gpiobug)

