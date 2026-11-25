#include <stdbool.h>
#include <stdint.h>

#include "bsp_adc.h"
#include "bsp_clock.h"
#include "bsp_init.h"
#include "bsp_pinout.h"
#include "bsp_uart.h"
#include "project_config.h"
#include "stm32f1xx_ll_usart.h"

// ============================================================================
// ТЕСТОВЫЙ MAIN: замер пачки ADC (4 канала × 300 циклов) и вывод средних
// ============================================================================

// Заглушки UART-коллбэков (эхо не используем)
static void on_rx(uint8_t byte) {
    (void)byte;
}
static bool on_tx_next(uint8_t *out_byte) {
    (void)out_byte;
    return false;
}
static void on_idle(void) {}

// ============================================================================
// ОТПРАВКА СТРОКИ ЧЕРЕЗ USART1 (блокирующая, только для теста!)
// ============================================================================
static void uart_send_string(const char *s) {
    while (*s) {
        while (LL_USART_IsActiveFlag_TXE(BSP_UART_NEXTION) == 0U) {
        }
        LL_USART_TransmitData8(BSP_UART_NEXTION, (uint8_t)*s++);
    }
    while (LL_USART_IsActiveFlag_TC(BSP_UART_NEXTION) == 0U) {
    }
}

// ============================================================================
// ВЫВОД БЕЗЗНАКОВОГО ЧИСЛА В UART (без sprintf)
// ============================================================================
static void uart_send_uint(uint32_t v) {
    char buf[12];
    int i = 0;
    if (v == 0U) {
        buf[i++] = '0';
    } else {
        char tmp[10];
        int j = 0;
        while (v > 0U) {
            tmp[j++] = (char)('0' + (v % 10U));
            v /= 10U;
        }
        while (j > 0) {
            buf[i++] = tmp[--j];
        }
    }
    buf[i] = '\0';
    uart_send_string(buf);
}

// ============================================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ============================================================================
int main(void) {
    // 0. Безопасное состояние выходов
    BSP_SafeOutputsInit();

    // 1. Полная инициализация BSP
    BSP_SystemInit();

    // 2. Регистрация UART-коллбэков (заглушки)
    BSP_Uart_SetCallbacks(BSP_UART_ID_NEXTION, on_rx, on_tx_next, on_idle);

    // 3. Приветствие
    uart_send_string("\r\nADC batch test start\r\n");
    uart_send_string("4 channels x 300 cycles, 239.5 sampling cycles\r\n");

    // 4. Основной цикл: раз в 1 секунду запускаем пачку и выводим средние
    uint32_t last_batch_ms = 0;

    while (1) {
        uint32_t now = BSP_GetTickMs();

        if ((now - last_batch_ms) >= 1000U) {
            last_batch_ms = now;

            // Запуск пачки (ADC continuous, DMA normal, 1200 отсчётов)
            BSP_ADC_StartBatch();

            // Ждём завершения (с таймаутом 100 мс)
            uint32_t t_start = BSP_GetTickMs();
            bool done = false;
            while ((BSP_GetTickMs() - t_start) < 100U) {
                if (BSP_ADC_IsBatchDone()) {
                    done = true;
                    break;
                }
                BSP_Watchdog_Feed();
            }

            if (!done) {
                uart_send_string("ADC: TIMEOUT\r\n");
            } else {
                // ADC и DMA уже остановлены в ISR.
                // Буфер: 1200 отсчётов, 4 канала, шаг 4.
                // WQ:        buf[0], buf[4], buf[8], ...
                // PRESS_4_20: buf[1], buf[5], buf[9], ...
                // PRESS_MAP:  buf[2], buf[6], buf[10], ...
                // TEMP_1:     buf[3], buf[7], buf[11], ...
                const uint16_t *buf = BSP_ADC_GetBatchBuffer();

                // Считаем средние "на лету"
                uint32_t sum_wq = 0U, sum_press4 = 0U, sum_pressm = 0U, sum_temp = 0U;
                for (uint16_t i = 0U; i < BSP_ADC_BATCH_CYCLES; i++) {
                    sum_wq += buf[i * 4U + 0U];
                    sum_press4 += buf[i * 4U + 1U];
                    sum_pressm += buf[i * 4U + 2U];
                    sum_temp += buf[i * 4U + 3U];
                }

                uint16_t avg_wq = (uint16_t)(sum_wq / BSP_ADC_BATCH_CYCLES);
                uint16_t avg_press4 = (uint16_t)(sum_press4 / BSP_ADC_BATCH_CYCLES);
                uint16_t avg_pressm = (uint16_t)(sum_pressm / BSP_ADC_BATCH_CYCLES);
                uint16_t avg_temp = (uint16_t)(sum_temp / BSP_ADC_BATCH_CYCLES);

                // Вывод: WQ:<v> P4:<v> PM:<v> T:<v>\r\n
                uart_send_string("WQ:");
                uart_send_uint(avg_wq);
                uart_send_string(" P4:");
                uart_send_uint(avg_press4);
                uart_send_string(" PM:");
                uart_send_uint(avg_pressm);
                uart_send_string(" T:");
                uart_send_uint(avg_temp);
                uart_send_string("\r\n");
            }
        }

        BSP_Watchdog_Feed();
    }
}