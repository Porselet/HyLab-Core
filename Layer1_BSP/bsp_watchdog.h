#ifndef BSP_WATCHDOG_H
#define BSP_WATCHDOG_H

#include <stdint.h>

// ============================================================================
// АППАРАТНЫЙ СТОРОЖЕВОЙ ТАЙМЕР IWDG
// ============================================================================
// Инициализация доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
// timeout_ms: желаемый период срабатывания (~500 мс по требованиям безопасности).
#ifdef BSP_INIT_INTERNAL
void BSP_Watchdog_Start(uint32_t timeout_ms);
#endif

// Сброс счётчика IWDG. Вызывается ТОЛЬКО в конце главного цикла while(1).
// Запрещено вызывать из прерываний!
void BSP_Watchdog_Feed(void);

#endif  // BSP_WATCHDOG_H