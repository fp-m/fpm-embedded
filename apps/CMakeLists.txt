#
# Set CC and link options for *.exe target.
#
function(fpm_target_options arg)
    target_include_directories(${arg} BEFORE PUBLIC
        ../../include
    )
    set_target_properties(${arg} PROPERTIES SUFFIX ".exe")
    target_compile_options(${arg} PRIVATE -fPIC -fno-asynchronous-unwind-tables)
    target_link_options(${arg} PRIVATE -shared -fPIC -nostartfiles -e main -B$ENV{HOME}/.local/lib/fpm/arm)

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
