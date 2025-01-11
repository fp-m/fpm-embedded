add_library(fpm_fatfs STATIC
    fatfs.c
    unicode.c
    strerror.c
)
target_include_directories(fpm_fatfs BEFORE PUBLIC
    ../include
)
