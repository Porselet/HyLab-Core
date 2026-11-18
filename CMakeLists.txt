cmake_minimum_required(VERSION 3.20)

# 1. Подключаем тулчейн СТРОГО ДО project()
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake")

project(HyLab_Core C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Дефайн семейства High-Density для STM32F103VET6 (512 КБ Flash, 64 КБ RAM)
add_compile_definitions(
    STM32F103xE
)

# 2. Пути к заголовкам проекта и драйверов
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

# 3. Автоматический сбор всех .c файлов из наших 4 этажей
file(GLOB_RECURSE APP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer1_BSP/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer2_System/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer3_Services/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Layer4_App/*.c"
)

# Системные файлы ST: инициализация ядра, стартап и утилиты задержек/PLL
set(SYS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xe.s
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_LL_Driver/Src/stm32f1xx_ll_utils.c
)

# Отключаем предупреждения о неиспользуемых параметрах внутри библиотечного файла ST
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_LL_Driver/Src/stm32f1xx_ll_utils.c
    PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter"
)

# 4. Сборка исполняемого ELF-файла
add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${SYS_SOURCES}
)

# 5. Подключение модифицированного линкер-скрипта (508K Flash + 4K EEPROM_EMU)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/STM32F103XE_FLASH.ld")
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map
)

# 6. Генерация .bin / .hex и вывод статистики памяти после сборки
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
)
