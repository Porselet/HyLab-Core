#ifndef BSP_TICK_H
#define BSP_TICK_H

#include <stdint.h>

// ============================================================================
// СИСТЕМНОЕ ВРЕМЯ: SysTick 1 мс, BSP_GetTickMs, BSP_DelayMs
// ============================================================================
// Инициализация доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
#ifdef BSP_INIT_INTERNAL
void BSP_Tick_Init(void);
#endif

// Системное время в миллисекундах с момента старта (монотонно растёт)
uint32_t BSP_GetTickMs(void);

// Блокирующая задержка (ТОЛЬКО для инициализации, запрещена в основном цикле)
void BSP_DelayMs(uint32_t ms);

#endif  // BSP_TICK_H