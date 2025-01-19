add_executable(free
    free.c
)
target_include_directories(free BEFORE PUBLIC
    ../../include
)
set_target_properties(free PROPERTIES SUFFIX ".exe")
target_compile_options(free PRIVATE -fPIC)
target_link_options(free PRIVATE -shared -fPIC -nostartfiles -e main -B$ENV{HOME}/.local/lib/fpm/arm)
