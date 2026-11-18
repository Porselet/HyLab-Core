#ifndef BSP_FLASH_H
#define BSP_FLASH_H

// Аппаратный драйвер внутренней Flash на чистом CMSIS (стирание страниц и запись 16/32 бит)
#include <stdint.h>
#include <stdbool.h>
#include "project_config.h"

#define EEPROM_PAGE_0_ADDR      (0x0807F000UL)
#define EEPROM_PAGE_1_ADDR      (0x0807F800UL)
#define EEPROM_PAGE_SIZE        (2048UL)

#endif // BSP_FLASH_H
