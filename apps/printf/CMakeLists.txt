add_executable(printf
    printf.c
)
fpm_target_options(printf)

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
