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
# See the official documentation https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html
# for guidance on setting up the Pico C/C++ SDK.
#

cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(rit-v300 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

## PIO USB Library Setup
set(PICO_PIO_USB_DIR ${CMAKE_CURRENT_LIST_DIR}/Pico-PIO-USB)

add_library(Pico-PIO-USB STATIC
  ${PICO_PIO_USB_DIR}/src/pio_usb.c
  ${PICO_PIO_USB_DIR}/src/pio_usb_host.c
  ${PICO_PIO_USB_DIR}/src/usb_crc.c
)
pico_generate_pio_header(Pico-PIO-USB ${PICO_PIO_USB_DIR}/src/usb_tx.pio)
pico_generate_pio_header(Pico-PIO-USB ${PICO_PIO_USB_DIR}/src/usb_rx.pio)

target_link_libraries(Pico-PIO-USB PRIVATE
  pico_stdlib
  pico_multicore
  hardware_pio
  hardware_dma
)
target_include_directories(Pico-PIO-USB PRIVATE ${PICO_PIO_USB_DIR})


add_executable(rit-v300 main.c cvideo.c graphics.c fonts.c terminal.c hid_app.c logo_vt_vectors.c vector.c
${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
)

pico_generate_pio_header(rit-v300 ${CMAKE_CURRENT_LIST_DIR}/cvideo_sync.pio)
pico_generate_pio_header(rit-v300 ${CMAKE_CURRENT_LIST_DIR}/cvideo_data.pio)

target_include_directories(rit-v300 PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${PICO_PIO_USB_DIR}/src)

target_link_libraries(
        rit-v300 PRIVATE
        pico_stdlib
        pico_mem_ops
        hardware_pio
        hardware_dma
        hardware_irq
        pico_bootrom
        tinyusb_host
        tinyusb_board   #needed for bsp/board_api.h
        Pico-PIO-USB
        
)

pico_add_extra_outputs(rit-v300)

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
