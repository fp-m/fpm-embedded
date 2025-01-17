add_executable(uf2fat
    diskio.cpp
    dump.cpp
    format.cpp
    main.cpp
    ../../fatfs/fatfs.c
    ../../fatfs/unicode.c
)
target_include_directories(uf2fat BEFORE PUBLIC
    ../../include
)
install(TARGETS uf2fat DESTINATION $ENV{HOME}/.local/bin)
