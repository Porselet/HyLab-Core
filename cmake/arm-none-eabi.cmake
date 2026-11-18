set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Отключаем попытки линковать тестовый .exe для Windows
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Ищем компилятор и утилиты в системном PATH
find_program(CMAKE_C_COMPILER NAMES arm-none-eabi-gcc REQUIRED)
find_program(CMAKE_ASM_COMPILER NAMES arm-none-eabi-gcc REQUIRED)
find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy REQUIRED)
find_program(CMAKE_SIZE NAMES arm-none-eabi-size REQUIRED)

# Базовые флаги ядра Cortex-M3 (без FPU)
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -Wall -Wextra -fdata-sections -ffunction-sections -O0 -g3" CACHE INTERNAL "")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS} -g3" CACHE INTERNAL "")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections" CACHE INTERNAL "")
