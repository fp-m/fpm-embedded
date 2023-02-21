add_library(rpm_fatfs STATIC
    fatfs.c
    unicode.c
    strerror.c
)
target_include_directories(rpm_fatfs BEFORE PUBLIC
    ../include
)
