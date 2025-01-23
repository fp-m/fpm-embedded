# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Build for rp2040
set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)

# Build for rp2350 in ARM mode
#set(PICO_BOARD pico2)
#set(PICO_PLATFORM rp2350-arm-s)

# Build for rp2350 in RISC-V mode
#set(PICO_BOARD pico2)
#set(PICO_PLATFORM rp2350-riscv)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(fpm-${PICO_PLATFORM} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    main_pico.c
    fpm_pico.c
    rtc_pico.c
    diskio.c
    flash.c
    sd_pico/crc.c
    sd_pico/sd_card.c
    sd_pico/sd_spi.c
    sd_pico/spi.c
)
add_subdirectory(../kernel kernel EXCLUDE_FROM_ALL)
add_subdirectory(../fatfs fatfs EXCLUDE_FROM_ALL)
add_subdirectory(../apps apps)

target_include_directories(${PROJECT_NAME} BEFORE PUBLIC
    ../include
    sd_pico
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_STACK_SIZE=32768
)

# Create uf2 file.
# Assume picotool is already installed.
set(picotool_FOUND 1)
pico_add_uf2_output(${PROJECT_NAME})
pico_add_dis_output(${PROJECT_NAME})

# Create .nm file.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-nm -n ${PROJECT_NAME}.elf > ${PROJECT_NAME}.nm
    VERBATIM
)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    fpm_kernel
    fpm_fatfs
    pico_stdlib
    hardware_dma
    hardware_spi
    hardware_flash
)
if(PICO_RP2040)
    target_link_libraries(${PROJECT_NAME} hardware_rtc)
endif()

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
set_source_files_properties(fpm_pico.c
    PROPERTIES COMPILE_FLAGS "-DGIT_REVCOUNT=\\\"${GIT_REVCOUNT}\\\" -DGIT_COMMIT=\\\"${GIT_COMMIT}\\\""
)

# Create build timestamp
string(TIMESTAMP BUILD_YEAR "%Y")
string(TIMESTAMP BUILD_MONTH "%m")
string(TIMESTAMP BUILD_DAY "%d")
string(TIMESTAMP BUILD_HOUR "%H")
string(TIMESTAMP BUILD_MIN "%M")
string(TIMESTAMP BUILD_SEC "%S")
string(TIMESTAMP BUILD_DOTW "%w")
set_source_files_properties(main_pico.c PROPERTIES COMPILE_FLAGS
    "-DBUILD_YEAR=${BUILD_YEAR} \
     -DBUILD_MONTH=${BUILD_MONTH} \
     -DBUILD_DAY=${BUILD_DAY} \
     -DBUILD_HOUR=${BUILD_HOUR} \
     -DBUILD_MIN=${BUILD_MIN} \
     -DBUILD_SEC=${BUILD_SEC} \
     -DBUILD_DOTW=${BUILD_DOTW}"
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

include(flash_image.cmake)

# 'make upload' to program the binary
add_custom_target(upload
    picotool load -f -x ${PROJECT_NAME}.uf2
    COMMENT "picotool load -f -x ${PROJECT_NAME}.uf2"
    DEPENDS ${PROJECT_NAME}
)

install(FILES
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2
    DESTINATION $ENV{HOME}/.local/lib/fpm
)
