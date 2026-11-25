#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// НАСТРОЙКА GPIO: входы, аналоговые пины, ремапы периферии
// ============================================================================
// Инициализация доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
// Верхние слои работают с пинами через bsp_pinout.h (BSP_Out_*, BSP_In_*).
#ifdef BSP_INIT_INTERNAL
void BSP_GPIO_Init(void);
#endif

#endif  // BSP_GPIO_H