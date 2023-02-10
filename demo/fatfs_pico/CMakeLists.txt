# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(fatfs_demo_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    main.c
    thing_plus.hw_config.c
)

# Build FatFs library
add_library(FatFs_SPI STATIC
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/ff15/source/ffsystem.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/ff15/source/ffunicode.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/ff15/source/ff.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/sd_driver/sd_spi.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/sd_driver/demo_logging.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/sd_driver/spi.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/sd_driver/sd_card.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/sd_driver/crc.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/src/glue.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/src/f_util.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/src/ff_stdio.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/src/my_debug.c
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/src/rtc.c
)
target_include_directories(FatFs_SPI BEFORE PUBLIC
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/ff15/source
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/sd_driver
    ${CMAKE_SOURCE_DIR}/../../kernel/fatfs/include
    $ENV{PICO_SDK_PATH}/src/common/pico_stdlib/include
    $ENV{PICO_SDK_PATH}/src/common/pico_base/include
    $ENV{PICO_SDK_PATH}/src/common/pico_sync/include
    $ENV{PICO_SDK_PATH}/src/common/pico_time/include
    $ENV{PICO_SDK_PATH}/src/common/pico_util/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/pico_platform/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/pico_stdio/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_timer/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_base/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_irq/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_uart/include
    $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include
    $ENV{PICO_SDK_PATH}/src/rp2040/hardware_structs/include
    $ENV{PICO_SDK_PATH}/src/rp2040/hardware_regs/include
    ${CMAKE_BINARY_DIR}/generated/pico_base
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    FatFs_SPI
    hardware_spi
    hardware_dma
    hardware_rtc
    pico_stdlib
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
