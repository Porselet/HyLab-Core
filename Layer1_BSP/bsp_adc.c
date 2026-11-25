// Единственный файл, которому разрешено видеть init-функции модулей BSP
#define BSP_INIT_INTERNAL
#include "bsp_adc.h"

#include "bsp_pinout.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_dma.h"

// ============================================================================
// ВНУТРЕННИЙ БУФЕР DMA: 1200 отсчётов (300 циклов × 4 канала)
// ============================================================================
static volatile uint16_t s_adc_dma_buf[BSP_ADC_BATCH_SAMPLES];

// Флаг завершения пачки (взводится в ISR DMA)
static volatile bool s_batch_done = false;

// Вспомогательная задержка стабилизации АЦП (~1-2 мкс при 72 МГц)
static void prv_adc_delay_us(void) {
    for (volatile uint32_t i = 0U; i < 100U; i++) {
        __NOP();
    }
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ ADC1 + DMA1 Channel1
// ============================================================================
void BSP_ADC_Init(void) {
    // 1. Тактирование ADC1 (APB2) и DMA1 (AHB)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    // 2. Включение АЦП и стабилизация
    LL_ADC_Enable(ADC1);
    prv_adc_delay_us();

    // 3. Калибровка (проводится строго до включения DMA и запуска конверсий)
    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1) != 0U) {
    }

    // 4. Настройка режима: SCAN (обязательно для F1!), CONTINUOUS, DMA, 4 канала
    LL_ADC_SetSequencersScanMode(ADC1, LL_ADC_SEQ_SCAN_ENABLE);
    LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
    LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
    LL_ADC_REG_SetSequencerLength(ADC1, LL_ADC_REG_SEQ_SCAN_ENABLE_4RANKS);

    // 5. Порядок каналов: WQ, PRESS_4_20, PRESS_MAP, TEMP_1
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_0);   // PA0 - WQ
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, LL_ADC_CHANNEL_11);  // PC1 - PRESS_4_20
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, LL_ADC_CHANNEL_10);  // PC0 - PRESS_MAP
    LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_4, LL_ADC_CHANNEL_12);  // PC2 - TEMP_1

    // 6. Время выборки для всех каналов: 239.5 циклов (максимум)
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_0, LL_ADC_SAMPLINGTIME_239CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_11, LL_ADC_SAMPLINGTIME_239CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_10, LL_ADC_SAMPLINGTIME_239CYCLES_5);
    LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_12, LL_ADC_SAMPLINGTIME_239CYCLES_5);

    // 7. Настройка DMA1 Channel1 (ADC1 -> память, NORMAL mode, 1200 отсчётов)
    LL_DMA_InitTypeDef dma = {0};
    dma.PeriphOrM2MSrcAddress = LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA);
    dma.MemoryOrM2MDstAddress = (uint32_t)s_adc_dma_buf;
    dma.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    dma.Mode = LL_DMA_MODE_NORMAL;
    dma.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
    dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dma.NbData = BSP_ADC_BATCH_SAMPLES;
    dma.Priority = LL_DMA_PRIORITY_HIGH;
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &dma);

    // 8. Прерывание DMA по завершении
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    NVIC_SetPriority(DMA1_Channel1_IRQn, 2U);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

// ============================================================================
// ЗАПУСК ПАЧКИ
// ============================================================================
void BSP_ADC_StartBatch(void) {
    // На случай, если предыдущая пачка не была остановлена корректно
    BSP_ADC_Stop();

    s_batch_done = false;

    // Восстановление счётчика транзакций и сброс всех флагов канала 1 (TC, HT, TE, GI)
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, BSP_ADC_BATCH_SAMPLES);
    LL_DMA_ClearFlag_GI1(DMA1);

    // Включение DMA до старта АЦП
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

    // Разрешение внешнего триггера (в STM32F1 SWSTART считается внешним триггером)
    LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

    // Запуск непрерывной конверсии
    LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
    LL_ADC_REG_StartConversionSWStart(ADC1);
}

// ============================================================================
// ОСТАНОВКА (аварийная и штатная)
// ============================================================================
void BSP_ADC_Stop(void) {
    // Выключаем АЦП: это единственно надёжный способ мгновенно прервать
    // текущее сканирование и сбросить указатель секвенсора на Rank 1
    LL_ADC_Disable(ADC1);

    // Остановка DMA
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    while (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_1) != 0U) {
    }
    LL_DMA_ClearFlag_GI1(DMA1);

    // Сразу включаем АЦП обратно и ждём стабилизации, чтобы он был готов к следующему старту
    LL_ADC_Enable(ADC1);
    prv_adc_delay_us();
}

// ============================================================================
// ПРОВЕРКА ЗАВЕРШЕНИЯ ПАЧКИ
// ============================================================================
bool BSP_ADC_IsBatchDone(void) {
    if (s_batch_done) {
        s_batch_done = false;
        return true;
    }
    return false;
}

// ============================================================================
// УКАЗАТЕЛЬ НА БУФЕР
// ============================================================================
const uint16_t *BSP_ADC_GetBatchBuffer(void) {
    return (const uint16_t *)s_adc_dma_buf;
}

// ============================================================================
// ОБРАБОТЧИК ПРЕРЫВАНИЯ DMA1 Channel1 (завершение пачки)
// ============================================================================
void DMA1_Channel1_IRQHandler(void) {
    if (LL_DMA_IsActiveFlag_TC1(DMA1) != 0U) {
        LL_DMA_ClearFlag_GI1(DMA1);

        // Переводим в одиночный режим: АЦП доделает текущий цикл из 4 каналов
        // (секвенсор вернётся на Rank 1) и остановится, не вызывая DMA (т.к. DMA уже выключен)
        LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_SINGLE);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);

        s_batch_done = true;
    }
}
