# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(rpm_shell_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wmissing-prototypes)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    main_pico.c
)
add_subdirectory(../../kernel kernel EXCLUDE_FROM_ALL)

target_include_directories(${PROJECT_NAME} BEFORE PUBLIC
    ../../include
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    rpm_kernel
    rpm_pico
    pico_stdlib
    hardware_rtc
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
