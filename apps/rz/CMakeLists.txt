add_executable(rz
    rz.c
    zserial.c
    zheaders.c
    znumbers.c
    crc.c
)
fpm_target_options(rz)

# Enable debug output.
target_compile_options(rz PRIVATE -DZDEBUG=1 -DZTRACE=1)
