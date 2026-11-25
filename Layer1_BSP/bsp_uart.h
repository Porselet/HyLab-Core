#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// АППАРАТНЫЕ UART-ПОРТЫ ПЛАТЫ (без DMA, с IDLE-прерыванием)
// ============================================================================

// Идентификаторы аппаратных UART-портов платы
typedef enum {
    BSP_UART_ID_NEXTION = 0,  // USART1 (PA9/PA10) - 9600 8N1
    BSP_UART_ID_SILISTLAB,    // USART2 (PD5/PD6 + PD4 DE) - 19200 9E1
    BSP_UART_ID_AUX_485,      // UART4  (PC10/PC11 + PA15 DE) - 9600 8N1
    BSP_UART_ID_MODBUS,       // UART5  (PC12/PD2 + PD1 DE) - 9600 8N1
    BSP_UART_ID_COUNT
} BspUartId_t;

// Коллбэки для связи с Layer2_System (uart_engine)
typedef void (*BspUartRxCallback_t)(uint8_t byte);
typedef bool (*BspUartTxNextByteCallback_t)(uint8_t *out_byte);
typedef void (*BspUartIdleCallback_t)(void);

// Инициализация доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
#ifdef BSP_INIT_INTERNAL
void BSP_Uart_Init(BspUartId_t id, uint32_t baudrate);
#endif

/**
 * @brief Регистрация обработчиков приёма, запроса следующего байта и IDLE.
 */
void BSP_Uart_SetCallbacks(BspUartId_t id, BspUartRxCallback_t on_rx, BspUartTxNextByteCallback_t on_tx_next,
                           BspUartIdleCallback_t on_idle);

/**
 * @brief Запуск передачи (переводит RS-485 в TX и включает TXEIE).
 * @return false, если передатчик занят или не зарегистрирован on_tx_next.
 */
bool BSP_Uart_StartTx(BspUartId_t id);

/**
 * @brief Проверка, занят ли передатчик.
 */
bool BSP_Uart_IsTxBusy(BspUartId_t id);

#endif  // BSP_UART_H