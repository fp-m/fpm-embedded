#
# Set CC and link options for *.exe target.
#
function(fpm_target_options arg)
    target_include_directories(${arg} BEFORE PUBLIC
        ../../include
    )
    set_target_properties(${arg} PROPERTIES SUFFIX ".exe")
    target_compile_options(${arg} PRIVATE -fPIC)
    target_link_options(${arg} PRIVATE -shared -fPIC -nostartfiles -e main -B$ENV{HOME}/.local/lib/fpm/arm)
endfunction()

add_subdirectory(hello)
add_subdirectory(cmd)
add_subdirectory(free)
add_subdirectory(printf)
