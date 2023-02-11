# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
#project(shell_pico C CXX ASM)
#set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 17)
#add_compile_options(-Wall -Werror)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Get the revision count
execute_process(
    COMMAND git rev-list HEAD --count
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_REVCOUNT
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_COMMIT
)

add_library(rpm_kernel STATIC
    rpm_editline.c
    rpm_getkey.c
    rpm_getwch.c
    rpm_puts.c
    rpm_putwch.c
    rpm_shell.c
    rpm_strlcpy.c
    rpm_strwlen.c
    rpm_tokenize.c
    rpm_wputs.c

    cmd/cmd_clear.c
    cmd/cmd_date.c
    cmd/cmd_echo.c
    cmd/cmd_help.c
    cmd/cmd_reboot.c
    cmd/cmd_time.c
    cmd/cmd_ver.c
)
target_include_directories(rpm_kernel BEFORE PUBLIC
    ../include
)

add_library(rpm_unix STATIC
    rpm_unix.c
)
#include_directories(rpm_unix BEFORE PUBLIC
#    ../include
#)

add_library(rpm_pico STATIC
    rpm_pico.c
)
target_include_directories(rpm_pico BEFORE PUBLIC
    $ENV{PICO_SDK_PATH}/src/rp2_common/pico_stdio_usb/include
)
target_link_libraries(rpm_pico
    rpm_kernel
    pico_stdlib
    hardware_rtc
)

set_source_files_properties(rpm_pico.c rpm_unix.c
    PROPERTIES COMPILE_FLAGS "-DGIT_REVCOUNT=\\\"${GIT_REVCOUNT}\\\" -DGIT_COMMIT=\\\"${GIT_COMMIT}\\\""
)
