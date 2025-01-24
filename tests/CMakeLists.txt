# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set name of project and C/C++ standards
project(fpm-test C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

add_subdirectory(../kernel kernel EXCLUDE_FROM_ALL)
add_subdirectory(../fatfs fatfs EXCLUDE_FROM_ALL)
add_subdirectory(../tools/elfexe elfexe)

#
# Download GoogleTest
#
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)
enable_testing()

#
# Common includes and libraries for all tests.
#
include_directories(BEFORE ../include)
link_libraries(gtest gtest_main)

#
# Check fpm_editline() routine.
#
add_executable(editline_tests
    editline_test.cpp
    ../kernel/fpm_editline.c
    ../kernel/fpm_puts.c
    ../kernel/fpm_wputs.c
    ../kernel/fpm_strwlen.c
    ../kernel/fpm_getwch.c
    ../kernel/fpm_getkey.c
    ../kernel/fpm_putwch.c
    ../kernel/fpm_strlcpy.c
)
gtest_discover_tests(editline_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check fpm_tokenize() routine.
#
add_executable(tokenize_tests
    tokenize_test.cpp
    ../kernel/fpm_tokenize.c
)
gtest_discover_tests(tokenize_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check fpm_getopt() routine.
#
add_executable(getopt_tests
    getopt_test.cpp
    ../kernel/fpm_getopt.c
)
gtest_discover_tests(getopt_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check fpm_get_dotw() routine.
#
add_executable(dotw_tests
    dotw_test.cpp
    ../kernel/fpm_get_dotw.c
)
gtest_discover_tests(dotw_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check fpm_fatfs() routine.
#
add_executable(fatfs_tests
    fatfs_test.cpp
    ../fatfs/fatfs.c
    ../fatfs/unicode.c
)
gtest_discover_tests(fatfs_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check cp/copy command.
#
add_executable(copy_tests
    copy_test.cpp
    fs_util.cpp
    console_util.cpp
    ../fatfs/fatfs.c
    ../fatfs/unicode.c
    ../kernel/cmd/cmd_copy.c
)
target_link_libraries(copy_tests
    fpm_kernel
    fpm_fatfs
)
gtest_discover_tests(copy_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check memory allocation: fpm_alloc() and others.
#
add_executable(alloc_tests
    alloc_test.cpp
    console_util.cpp
    ../kernel/fpm_alloc.c
)
gtest_discover_tests(alloc_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check dynamic loader.
#
add_executable(loader_tests
    loader_test.cpp
    console_util.cpp
    ../kernel/fpm_loader.c
    ../unix/loader_unix.c
)
gtest_discover_tests(loader_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

add_executable(loader2_tests
    loader2_test.cpp
    console_util.cpp
    ../kernel/fpm_loader.c
    ../unix/loader_unix.c
)
gtest_discover_tests(loader2_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check Zmodem protocol.
#
add_executable(zmodem_tests
    zmodem_test.cpp
    console_util.cpp
)
target_include_directories(zmodem_tests BEFORE PUBLIC
    ../apps/rz
)
gtest_discover_tests(zmodem_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

# Create -I options for exe build.
set(EXE_INCLUDES -I${CMAKE_SOURCE_DIR}/../include)
if (APPLE)
    # Get MacOS SDK path
    execute_process(COMMAND xcrun --show-sdk-path
                    OUTPUT_VARIABLE SDK_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND EXE_INCLUDES -I${SDK_PATH}/usr/include)

    # Set toolchain prefix.
    set(P x86_64-elf-)
endif()

#
# Build exe binaries for tests.
#
add_custom_command(OUTPUT hello.exe
    COMMAND ${P}gcc -fPIC -g -O1 -c ${EXE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/hello.c -o hello.o
    COMMAND ${P}ld -shared -fPIC -g -e main hello.o -o hello.exe
    COMMAND elfexe/elfexe hello.exe
    COMMAND ${P}objdump -d hello.exe > hello.dis
    COMMAND ${P}nm -n hello.exe > hello.nm
    COMMAND ${P}readelf -a hello.exe > hello.readelf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/hello.c elfexe
    VERBATIM
)
add_custom_command(OUTPUT testputs.exe
    COMMAND ${P}gcc -fPIC -g -O1 -c ${EXE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/testputs.c -o testputs.o
    COMMAND ${P}ld -shared -fPIC -g -e main testputs.o -o testputs.exe
    COMMAND elfexe/elfexe testputs.exe
    COMMAND ${P}objdump -d testputs.exe > testputs.dis
    COMMAND ${P}nm -n testputs.exe > testputs.nm
    COMMAND ${P}readelf -a testputs.exe > testputs.readelf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/testputs.c elfexe
    VERBATIM
)
add_custom_target(generate-exe ALL DEPENDS hello.exe testputs.exe)
