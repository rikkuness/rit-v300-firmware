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
set(PICO_BOARD adafruit_feather_rp2350 CACHE STRING "Board type")

#
# Title:	        Pico-mposite Makefile
# Description:		Makefile 
# Author:	        Dean Belfield
# Created:	        31/01/2021
# Last Updated:		26/09/2024
#
# Modinfo:
# 01/02/2022:		Added this header comment, fixed typo in executable filename, added extra target sources
# 19/02/2022:		Added terminal.c
# 26/09/2024:		Updated build files so that the project can be built more easily

#
# See the official documentation https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html
# for guidance on setting up the Pico C/C++ SDK.
#

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(pico-mposite C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(pico-mposite main.c cvideo.c graphics.c charset.c terminal.c)

pico_generate_pio_header(pico-mposite ${CMAKE_CURRENT_LIST_DIR}/cvideo_sync.pio)
pico_generate_pio_header(pico-mposite ${CMAKE_CURRENT_LIST_DIR}/cvideo_data.pio)

target_link_libraries(
        pico-mposite PRIVATE
        pico_stdlib
        pico_mem_ops
        hardware_pio
        hardware_dma
        hardware_irq
        pico_bootrom
)

pico_add_extra_outputs(pico-mposite)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/cvideo_sync.pio.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/cvideo_sync.pio
        COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/cvideo_sync.pio ${CMAKE_CURRENT_LIST_DIR}/generated/cvideo_sync.pio.h
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/cvideo_data.pio.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/cvideo_data.pio
        COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/cvideo_data.pio ${CMAKE_CURRENT_LIST_DIR}/generated/cvideo_data.pio.h
)
