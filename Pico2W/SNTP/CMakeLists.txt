# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

include(pico_sdk_import.cmake)

project(sntp_project)

pico_sdk_init()

add_executable(sntp
    main.c
    lcd.c
    sntp.c
)

target_include_directories(sntp PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(sntp PRIVATE
    pico_cyw43_arch_lwip_poll
    hardware_i2c
    pico_stdlib
)

pico_enable_stdio_usb(sntp 0)
pico_enable_stdio_uart(sntp 1)

pico_add_extra_outputs(sntp)

