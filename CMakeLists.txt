cmake_minimum_required(VERSION 3.22)

# Подключаем тулчейн ДО команды project()
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake")

project(HyLab-Core C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Глобальные определения для STM32F103VET6, LL-драйверов и кварца 16 МГц
add_compile_definitions(
    STM32F103xE
    USE_FULL_LL_DRIVER
    HSE_VALUE=16000000U
)

# Пути к заголовочным файлам
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Config
    ${CMAKE_SOURCE_DIR}/Layer1_BSP
    ${CMAKE_SOURCE_DIR}/Layer2_System
    ${CMAKE_SOURCE_DIR}/Layer3_Services
    ${CMAKE_SOURCE_DIR}/Layer4_App
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_LL_Driver/Inc
)

# Все исходные файлы проекта
file(GLOB_RECURSE PROJECT_SOURCES
    "Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c"
    "Layer1_BSP/*.c"
    "Layer2_System/*.c"
    "Layer3_Services/*.c"
    "Layer4_App/*.c"
    "Drivers/STM32F1xx_LL_Driver/Src/*.c"
)

# Стартап-файл и линкер-скрипт
set(STARTUP_FILE "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xe.s")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F103VETx_FLASH.ld")

# Собираем исполняемый ELF-файл
add_executable(${PROJECT_NAME}.elf ${PROJECT_SOURCES} ${STARTUP_FILE})

# Флаги линковщика (подключаем скрипт памяти, math-библиотеку и newlib-nano)
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -Wl,--no-warn-rwx-segments
    -lc -lm
    
)


# Автоматическая конвертация в .hex и .bin после успешной компиляции
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating HEX, BIN and checking flash/RAM usage..."
)
