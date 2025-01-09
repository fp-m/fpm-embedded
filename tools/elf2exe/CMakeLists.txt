add_executable(elf2exe elf2exe.c)
target_include_directories(elf2exe BEFORE PUBLIC
    ../../include
)
