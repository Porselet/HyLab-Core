#ifndef BSP_CLOCK_H
#define BSP_CLOCK_H

#include <stdint.h>

// ============================================================================
// ТАКТИРОВАНИЕ СИСТЕМЫ: HSE 16 МГц -> PLL (/2 * 9) = 72 МГц
// ============================================================================
// Инициализация доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
// Верхние слои не должны менять тактирование.
#ifdef BSP_INIT_INTERNAL
void BSP_Clock_Init72MHz(void);
#endif

// Текущая системная частота (Гц). Доступна всем слоям для расчётов.
uint32_t BSP_Clock_GetSysFreq(void);

#endif  // BSP_CLOCK_H