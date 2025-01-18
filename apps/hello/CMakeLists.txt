add_executable(hello
    hello.c
)
target_include_directories(hello BEFORE PUBLIC
    ../../include
)
set_target_properties(hello PROPERTIES SUFFIX ".exe")
target_compile_options(hello PRIVATE -fPIC)
target_link_options(hello PRIVATE -shared -fPIC -nostdlib -e main -B$ENV{HOME}/.local/lib/fpm/arm)
