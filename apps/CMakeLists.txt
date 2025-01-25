#
# Set CC and link options for *.exe target.
#
function(fpm_target_options arg)
    target_include_directories(${arg} BEFORE PUBLIC
        ../../include
        $ENV{PICO_SDK_PATH}/src/common/pico_base_headers/include
        $ENV{PICO_SDK_PATH}/src/common/pico_stdlib_headers/include
        $ENV{PICO_SDK_PATH}/src/common/pico_time/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/pico_stdio/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_base/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_timer/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_gpio/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_irq/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_uart/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_resets/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
        $ENV{PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include
        $ENV{PICO_SDK_PATH}/src/rp2040/hardware_structs/include
        $ENV{PICO_SDK_PATH}/src/rp2040/pico_platform/include
        $ENV{PICO_SDK_PATH}/src/rp2040/hardware_regs/include
        ${CMAKE_BINARY_DIR}/generated/pico_base
    )
    set_target_properties(${arg} PROPERTIES SUFFIX ".exe")
    target_compile_options(${arg} PRIVATE -fPIC -fno-asynchronous-unwind-tables -DPICO_RP2040=1)
    target_link_options(${arg} PRIVATE -shared -fPIC -Wl,--version-script=${CMAKE_SOURCE_DIR}/../apps/export.sym -nostartfiles -e main -B$ENV{HOME}/.local/lib/fpm/arm)

    #
    # Disassemble
    #
    add_custom_command(OUTPUT ${arg}.dis DEPENDS ${arg}
        COMMAND arm-none-eabi-objdump -d ${arg}.exe > ${arg}.dis
    )
    add_custom_command(OUTPUT ${arg}.nm DEPENDS ${arg}
        COMMAND arm-none-eabi-nm -n ${arg}.exe > ${arg}.nm
    )
    add_custom_command(OUTPUT ${arg}.readelf DEPENDS ${arg}
        COMMAND arm-none-eabi-readelf -a ${arg}.exe > ${arg}.readelf
    )
    add_custom_target(${arg}-objdump ALL DEPENDS
        ${arg}.dis
        ${arg}.nm
        ${arg}.readelf
    )
endfunction()

add_subdirectory(hello)
add_subdirectory(cmd)
add_subdirectory(free)
add_subdirectory(printf)
add_subdirectory(gpio)
add_subdirectory(rz)
