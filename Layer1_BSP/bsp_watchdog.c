#define BSP_INIT_INTERNAL
#include "bsp_watchdog.h"

#include "stm32f1xx_ll_iwdg.h"

// LSI на STM32F103 ~40 кГц (по даташиту 30..60 кГц).
// Для периода ~500 мс при LSI 40 кГц:
//   PRESCALER_64 -> 40 000 / 64 = 625 Гц (тик 1.6 мс)
//   Reload = 500 мс / 1.6 мс = 312
// При разбросе LSI 30..60 кГц реальный период: 333..666 мс.
#define IWDG_LSI_FREQ_HZ 40000U
#define IWDG_PRESCALER 64U
#define IWDG_TICK_HZ (IWDG_LSI_FREQ_HZ / IWDG_PRESCALER)

// ============================================================================
// ЗАПУСК IWDG С ЗАДАННЫМ ПЕРИОДОМ
// ============================================================================
#ifdef BSP_INIT_INTERNAL
void BSP_Watchdog_Start(uint32_t timeout_ms) {
    // Расчёт значения reload из желаемого периода
    uint32_t reload = (timeout_ms * IWDG_TICK_HZ) / 1000U;

    // Ограничение диапазона reload (12-битный регистр: 1..4095)
    if (reload < 1U) {
        reload = 1U;
    }
    if (reload > 4095U) {
        reload = 4095U;
    }

    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_64);
    LL_IWDG_SetReloadCounter(IWDG, reload);
    while (LL_IWDG_IsReady(IWDG) != 1U) {
    }
    LL_IWDG_ReloadCounter(IWDG);
}
#endif  // BSP_INIT_INTERNAL

// ============================================================================
// СБРОС СЧЁТЧИКА IWDG
// ============================================================================
void BSP_Watchdog_Feed(void) {
    LL_IWDG_ReloadCounter(IWDG);
}