add_executable(rz
    rz.c
    zserial.c
    zheaders.c
    znumbers.c
    crc.c
)
fpm_target_options(rz)
