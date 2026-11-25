#ifndef BSP_ADC_H
#define BSP_ADC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef BSP_INIT_INTERNAL
void BSP_ADC_Init(void);
#endif

/**
 * @brief Запуск пачки из BSP_ADC_BATCH_CYCLES полных циклов (4 канала).
 *        DMA в normal mode, по завершении — прерывание TC.
 */
void BSP_ADC_StartBatch(void);

/**
 * @brief Остановка DMA и АЦП (если пачка идёт).
 */
void BSP_ADC_Stop(void);

/**
 * @brief Проверка завершения пачки (сбрасывает флаг).
 */
bool BSP_ADC_IsBatchDone(void);

/**
 * @brief Указатель на буфер пачки (1200 отсчётов).
 */
const uint16_t *BSP_ADC_GetBatchBuffer(void);

#endif  // BSP_ADC_H