#define BSP_INIT_INTERNAL
#include "bsp_clock.h"

#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"

// Системная частота (обновляется при инициализации)
static uint32_t s_sys_freq_hz = 8000000U;  // до init — HSI 8 МГц

// ============================================================================
// ТАКТИРОВАНИЕ: HSE 16 МГц -> PLL (/2 * 9) = 72 МГц
// ============================================================================
#ifdef BSP_INIT_INTERNAL
void BSP_Clock_Init72MHz(void) {
    // 2 цикла ожидания Flash для частоты > 48 МГц
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
    }

    // Запуск внешнего кварцевого генератора HSE (16 МГц)
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1U) {
    }

    // Настройка PLL: источник HSE/2 (8 МГц) * 9 = 72 МГц
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_2, LL_RCC_PLL_MUL_9);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1U) {
    }

    // Делители шин: AHB = 72 МГц, APB1 = 36 МГц (таймеры *2 = 72 МГц), APB2 = 72 МГц
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    // Делитель тактирования АЦП: PCLK2 (72 МГц) / 6 = 12 МГц (макс. допустимо 14 МГц)
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_6);

    // Переключение системной шины на PLL
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }

    s_sys_freq_hz = 72000000U;
    SystemCoreClock = 72000000U;
    LL_SetSystemCoreClock(SystemCoreClock);
}
#endif  // BSP_INIT_INTERNAL

// ============================================================================
// ТЕКУЩАЯ СИСТЕМНАЯ ЧАСТОТА
// ============================================================================
uint32_t BSP_Clock_GetSysFreq(void) {
    return s_sys_freq_hz;
}