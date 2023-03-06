# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set name of project and C/C++ standards
project(rpm-test C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

#
# Download GoogleTest
#
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()
include(GoogleTest)
enable_testing()

#
# Common includes and libraries for all tests.
#
include_directories(BEFORE ../include)
link_libraries(gtest gtest_main)

#
# Check rpm_editline() routine.
#
add_executable(editline_tests
    editline_test.cpp
    ../kernel/rpm_editline.c
    ../kernel/rpm_puts.c
    ../kernel/rpm_wputs.c
    ../kernel/rpm_strwlen.c
    ../kernel/rpm_getwch.c
    ../kernel/rpm_getkey.c
    ../kernel/rpm_putwch.c
    ../kernel/rpm_strlcpy.c
)
gtest_discover_tests(editline_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check rpm_tokenize() routine.
#
add_executable(tokenize_tests
    tokenize_test.cpp
    ../kernel/rpm_tokenize.c
)
gtest_discover_tests(tokenize_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check rpm_getopt() routine.
#
add_executable(getopt_tests
    getopt_test.cpp
    ../kernel/rpm_getopt.c
)
gtest_discover_tests(getopt_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check rpm_get_dotw() routine.
#
add_executable(dotw_tests
    dotw_test.cpp
    ../kernel/rpm_get_dotw.c
)
gtest_discover_tests(dotw_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)

#
# Check rpm_fatfs() routine.
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
    ../fatfs/fatfs.c
    ../fatfs/unicode.c
)
gtest_discover_tests(copy_tests EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)
