add_executable(printf
    printf.c
)
target_include_directories(printf BEFORE PUBLIC
    ../../include
)
set_target_properties(printf PROPERTIES SUFFIX ".exe")
target_compile_options(printf PRIVATE -fPIC)
target_link_options(printf PRIVATE -shared -fPIC -nostdlib -e main -B$ENV{HOME}/.local/lib/fpm/arm)
