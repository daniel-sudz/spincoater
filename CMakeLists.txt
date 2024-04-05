cmake_minimum_required(VERSION 3.13)

# include the pico SDK
if (DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
else ()
    set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)
endif ()

# set the arm toolchain path
if (DEFINED ENV{PICO_TOOLCHAIN_PATH})
    set(PICO_TOOLCHAIN_PATH $ENV{PICO_TOOLCHAIN_PATH})
else ()
    set(PICO_TOOLCHAIN_PATH "/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi")
endif ()

# run pico sdk setup
include(pico_sdk_import.cmake)

# everything for our project is below
project(spincoater)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# build and link the project
add_executable(main
    main.cpp
)

target_link_libraries(main pico_stdlib)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)

# enable USB output, disable UART output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)