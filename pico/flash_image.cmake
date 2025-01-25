#
# Build contents of Flash filesystem
#
add_custom_command(OUTPUT flashfs
    COMMENT "Build flash contents"
    DEPENDS cmd.exe
            free.exe
            hello.exe
            printf.exe
            gpio.exe
            rz.exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/flashfs
                flashfs
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/cmd/cmd.exe
                flashfs/bin/cmd.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/free/free.exe
                flashfs/bin/free.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/hello/hello.exe
                flashfs/bin/hello.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/printf/printf.exe
                flashfs/bin/printf.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/gpio/gpio.exe
                flashfs/bin/gpio.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/rz/rz.exe
                flashfs/bin/rz.exe
    COMMAND arm-none-eabi-strip flashfs/bin/*.exe
)

set(UF2FAT ${CMAKE_SOURCE_DIR}/../unix/build/uf2fat/uf2fat)

#
# Create images for Flash sizes 2/8/16 Mbytes
#
add_custom_command(OUTPUT ${PROJECT_NAME}-2mb.uf2
    COMMENT "Build 2-Mb Flash image"
    DEPENDS ${PROJECT_NAME} flashfs
    COMMAND ${UF2FAT} format ${PROJECT_NAME}.uf2 2m flashfs -o ${PROJECT_NAME}-2mb.uf2
)
add_custom_command(OUTPUT ${PROJECT_NAME}-8mb.uf2
    COMMENT "Build 8-Mb Flash image"
    DEPENDS ${PROJECT_NAME} flashfs
    COMMAND ${UF2FAT} format ${PROJECT_NAME}.uf2 8m flashfs -o ${PROJECT_NAME}-8mb.uf2
)
add_custom_command(OUTPUT ${PROJECT_NAME}-16mb.uf2
    COMMENT "Build 16-Mb Flash image"
    DEPENDS ${PROJECT_NAME} flashfs
    COMMAND ${UF2FAT} format ${PROJECT_NAME}.uf2 16m flashfs -o ${PROJECT_NAME}-16mb.uf2
)
add_custom_target(images ALL DEPENDS
    ${PROJECT_NAME}-2mb.uf2
    ${PROJECT_NAME}-8mb.uf2
    ${PROJECT_NAME}-16mb.uf2
)
