#ifndef BSP_INIT_H
#define BSP_INIT_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// ФАСАД ИНИЦИАЛИЗАЦИИ BSP
// ============================================================================
// BSP_SafeOutputsInit() — вызывается из SystemInit() ДО main().
// BSP_SystemInit()     — вызывается первым в main(), инициализирует всё железо.

void BSP_SafeOutputsInit(void);
void BSP_SystemInit(void);

// ============================================================================
// РЕЭКСПОРТ ПУБЛИЧНОГО API (для удобства main.c)
// ============================================================================

#include "bsp_tick.h"      // BSP_GetTickMs, BSP_DelayMs
#include "bsp_watchdog.h"  // BSP_Watchdog_Feed

#endif  // BSP_INIT_H