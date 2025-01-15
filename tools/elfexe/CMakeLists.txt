add_executable(elfexe elfexe.c)
target_include_directories(elfexe BEFORE PUBLIC
    ../../include
)
