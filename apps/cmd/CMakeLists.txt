add_executable(cmd
    cmd.c
)
target_include_directories(cmd BEFORE PUBLIC
    ../../include
)
set_target_properties(cmd PROPERTIES SUFFIX ".exe")
target_compile_options(cmd PRIVATE -fPIC)
target_link_options(cmd PRIVATE -shared -fPIC -nostdlib -e main -B$ENV{HOME}/.local/lib/fpm/arm)
