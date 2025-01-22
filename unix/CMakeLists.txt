# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(fpm-demo C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wshadow)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    main_unix.c
    fpm_unix.c
    diskio_unix.c
    loader_unix.c
    bindings.c
)
add_subdirectory(../kernel kernel EXCLUDE_FROM_ALL)
add_subdirectory(../fatfs fatfs EXCLUDE_FROM_ALL)
add_subdirectory(../tools/elfexe elfexe)
add_subdirectory(../tools/uf2fat uf2fat)

target_include_directories(${PROJECT_NAME} BEFORE PUBLIC
    ../include
)

target_link_libraries(${PROJECT_NAME}
    fpm_kernel
    fpm_fatfs
)

# Get git commit hash and revision count
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_COMMIT
)
execute_process(
    COMMAND git rev-list HEAD --count
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GIT_REVCOUNT
)
set_source_files_properties(fpm_unix.c
    PROPERTIES COMPILE_FLAGS "-DGIT_REVCOUNT=\\\"${GIT_REVCOUNT}\\\" -DGIT_COMMIT=\\\"${GIT_COMMIT}\\\""
)

install(TARGETS
    ${PROJECT_NAME}
    DESTINATION $ENV{HOME}/.local/bin
)
