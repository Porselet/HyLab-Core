cmake_minimum_required(VERSION 3.20)

# Подключаем тулчейн строго ДО project()
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake")

project(HyLab_Core C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Дефайн семейства High-Density для STM32F103VET6 (512 КБ Flash, 64 КБ RAM)
add_compile_definitions(
    STM32F103xE
    USE_FULL_LL_DRIVER
)

# Пути ко всем заголовкам проекта и драйверов
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Layer1_BSP
    ${CMAKE_CURRENT_SOURCE_DIR}/Layer2_System
    ${CMAKE_CURRENT_SOURCE_DIR}/Layer3_Services
    ${CMAKE_CURRENT_SOURCE_DIR}/Layer4_App
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include/m-profile
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_LL_Driver/Inc
)


# Автоматически собираем все .c файлы из наших слоев
file(GLOB_RECURSE APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer1_BSP/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer2_System/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer3_Services/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer4_App/*.c"
)

# Системные исходники ST и стартап для STM32F103xE
set(SYS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xe.s
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_LL_Driver/Src/stm32f1xx_ll_utils.c
)

add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${SYS_SOURCES}
)

# Скрипт линковщика для STM32F103xE (512K Flash / 64K RAM)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/linker/STM32F103XE_FLASH.ld")
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
)

# Генерация .bin и .hex + вывод размера прошивки
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
)
