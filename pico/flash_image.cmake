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
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/flashfs
                ${CMAKE_CURRENT_BINARY_DIR}/flashfs
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/cmd/cmd.exe
                ${CMAKE_CURRENT_BINARY_DIR}/flashfs/bin/cmd.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/free/free.exe
                ${CMAKE_CURRENT_BINARY_DIR}/flashfs/bin/free.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/hello/hello.exe
                ${CMAKE_CURRENT_BINARY_DIR}/flashfs/bin/hello.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/printf/printf.exe
                ${CMAKE_CURRENT_BINARY_DIR}/flashfs/bin/printf.exe
    COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_BINARY_DIR}/apps/gpio/gpio.exe
                ${CMAKE_CURRENT_BINARY_DIR}/flashfs/bin/gpio.exe
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
