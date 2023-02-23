# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(rpm_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    main_pico.c
    rpm_pico.c
    rtc_pico.c
    diskio.c
    sd_pico/crc.c
    sd_pico/demo_logging.c
    sd_pico/hw_config.c
    sd_pico/sd_card.c
    sd_pico/sd_spi.c
    sd_pico/spi.c
)
add_subdirectory(../kernel kernel EXCLUDE_FROM_ALL)
add_subdirectory(../fatfs fatfs EXCLUDE_FROM_ALL)

target_include_directories(${PROJECT_NAME} BEFORE PUBLIC
    ../include
    sd_pico
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    rpm_kernel
    rpm_fatfs
    pico_stdlib
    hardware_rtc
    hardware_dma
    hardware_spi
)

# Get git commit hash and revision count
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_COMMIT
)
execute_process(
    COMMAND git rev-list HEAD --count
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_REVCOUNT
)
set_source_files_properties(rpm_pico.c
    PROPERTIES COMPILE_FLAGS "-DGIT_REVCOUNT=\\\"${GIT_REVCOUNT}\\\" -DGIT_COMMIT=\\\"${GIT_COMMIT}\\\""
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# 'make upload' to program the binary
add_custom_target(upload
    picotool load -f -x ${PROJECT_NAME}.uf2
    COMMENT "picotool load -f -x ${PROJECT_NAME}.uf2"
    DEPENDS ${PROJECT_NAME}
)

install(FILES
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2
    DESTINATION share/rp-m
)
