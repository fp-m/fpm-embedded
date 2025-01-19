add_executable(printf
    printf.c
)
target_include_directories(printf BEFORE PUBLIC
    ../../include
)
set_target_properties(printf PROPERTIES SUFFIX ".exe")
target_compile_options(printf PRIVATE -fPIC)
target_link_options(printf PRIVATE -shared -fPIC -nostartfiles -e main -B$ENV{HOME}/.local/lib/fpm/arm)

#
# Disassemble
#
add_custom_command(OUTPUT printf.dis DEPENDS printf
    COMMAND arm-none-eabi-objdump -d printf.exe > printf.dis
)
add_custom_command(OUTPUT printf.nm DEPENDS printf
    COMMAND arm-none-eabi-nm -n printf.exe > printf.nm
)
add_custom_command(OUTPUT printf.readelf DEPENDS printf
    COMMAND arm-none-eabi-readelf -a printf.exe > printf.readelf
)
add_custom_target(printf-objdump ALL DEPENDS
    printf.dis
    printf.nm
    printf.readelf
)
