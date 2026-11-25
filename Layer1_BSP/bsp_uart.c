#define BSP_INIT_INTERNAL
#include "bsp_uart.h"

#include "bsp_pinout.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_usart.h"

// Контекст каждого аппаратного UART-порта
typedef struct {
    USART_TypeDef *instance;
    IRQn_Type irqn;
    BspUartRxCallback_t on_rx;
    BspUartTxNextByteCallback_t on_tx_next;
    BspUartIdleCallback_t on_idle;
    volatile bool tx_busy;
    bool is_rs485;
} BspUartCtx_t;

static BspUartCtx_t s_uart_ctx[BSP_UART_ID_COUNT] = {
    [BSP_UART_ID_NEXTION] = {.instance = BSP_UART_NEXTION, .irqn = USART1_IRQn, .is_rs485 = false},
    [BSP_UART_ID_SILISTLAB] = {.instance = BSP_UART_SILISTLAB, .irqn = USART2_IRQn, .is_rs485 = true},
    [BSP_UART_ID_AUX_485] = {.instance = BSP_UART_AUX, .irqn = UART4_IRQn, .is_rs485 = true},
    [BSP_UART_ID_MODBUS] = {.instance = BSP_UART_MODBUS, .irqn = UART5_IRQn, .is_rs485 = true}};

// ============================================================================
// УПРАВЛЕНИЕ ПИНОМ DE ДЛЯ RS-485
// ============================================================================
static inline void prv_SetRs485Dir(BspUartId_t id, bool tx_enable) {
    switch (id) {
        case BSP_UART_ID_SILISTLAB:
            BSP_RS485_SetTx2(tx_enable);
            break;
        case BSP_UART_ID_AUX_485:
            BSP_RS485_SetTx4(tx_enable);
            break;
        case BSP_UART_ID_MODBUS:
            BSP_RS485_SetTx5(tx_enable);
            break;
        default:
            break;
    }
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ ПОРТОВ UART / USART
// ============================================================================
#ifdef BSP_INIT_INTERNAL
void BSP_Uart_Init(BspUartId_t id, uint32_t baudrate) {
    if ((uint32_t)id >= BSP_UART_ID_COUNT) {
        return;
    }

    BspUartCtx_t *ctx = &s_uart_ctx[id];
    ctx->tx_busy = false;
    prv_SetRs485Dir(id, false);

    LL_GPIO_InitTypeDef gpio_tx = {
        .Mode = LL_GPIO_MODE_ALTERNATE, .Speed = LL_GPIO_SPEED_FREQ_HIGH, .OutputType = LL_GPIO_OUTPUT_PUSHPULL};
    LL_GPIO_InitTypeDef gpio_rx = {.Mode = LL_GPIO_MODE_FLOATING};

    LL_USART_InitTypeDef usart = {0};
    usart.BaudRate = baudrate;
    usart.DataWidth = LL_USART_DATAWIDTH_8B;
    usart.StopBits = LL_USART_STOPBITS_1;
    usart.Parity = LL_USART_PARITY_NONE;
    usart.TransferDirection = LL_USART_DIRECTION_TX_RX;
    usart.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart.OverSampling = LL_USART_OVERSAMPLING_16;

    switch (id) {
        case BSP_UART_ID_NEXTION:
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
            gpio_tx.Pin = PIN_USART1_TX_PIN;
            LL_GPIO_Init(PIN_USART1_TX_PORT, &gpio_tx);
            gpio_rx.Pin = PIN_USART1_RX_PIN;
            LL_GPIO_Init(PIN_USART1_RX_PORT, &gpio_rx);
            break;

        case BSP_UART_ID_SILISTLAB:
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
            gpio_tx.Pin = PIN_USART2_TX_PIN;
            LL_GPIO_Init(PIN_USART2_TX_PORT, &gpio_tx);
            gpio_rx.Pin = PIN_USART2_RX_PIN;
            LL_GPIO_Init(PIN_USART2_RX_PORT, &gpio_rx);
            // Протокол Silistlab: 9 бит (8 данных + бит паритета Even)
            usart.DataWidth = LL_USART_DATAWIDTH_9B;
            usart.Parity = LL_USART_PARITY_EVEN;
            break;

        case BSP_UART_ID_AUX_485:
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
            gpio_tx.Pin = PIN_UART4_TX_PIN;
            LL_GPIO_Init(PIN_UART4_TX_PORT, &gpio_tx);
            gpio_rx.Pin = PIN_UART4_RX_PIN;
            LL_GPIO_Init(PIN_UART4_RX_PORT, &gpio_rx);
            break;

        case BSP_UART_ID_MODBUS:
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
            gpio_tx.Pin = PIN_UART5_TX_PIN;
            LL_GPIO_Init(PIN_UART5_TX_PORT, &gpio_tx);
            gpio_rx.Pin = PIN_UART5_RX_PIN;
            LL_GPIO_Init(PIN_UART5_RX_PORT, &gpio_rx);
            break;

        default:
            return;
    }

    LL_USART_DeInit(ctx->instance);
    LL_USART_Init(ctx->instance, &usart);
    LL_USART_ConfigAsyncMode(ctx->instance);

    // Очистка флагов и разрешение прерываний: RXNE + IDLE
    LL_USART_ClearFlag_TC(ctx->instance);
    LL_USART_EnableIT_RXNE(ctx->instance);
    LL_USART_EnableIT_IDLE(ctx->instance);

    NVIC_SetPriority(ctx->irqn, 3U);
    NVIC_EnableIRQ(ctx->irqn);

    LL_USART_Enable(ctx->instance);
}
#endif  // BSP_INIT_INTERNAL

// ============================================================================
// РЕГИСТРАЦИЯ КОЛЛБЭКОВ
// ============================================================================
void BSP_Uart_SetCallbacks(BspUartId_t id, BspUartRxCallback_t on_rx, BspUartTxNextByteCallback_t on_tx_next,
                           BspUartIdleCallback_t on_idle) {
    if ((uint32_t)id >= BSP_UART_ID_COUNT) {
        return;
    }
    s_uart_ctx[id].on_rx = on_rx;
    s_uart_ctx[id].on_tx_next = on_tx_next;
    s_uart_ctx[id].on_idle = on_idle;
}

// ============================================================================
// ЗАПУСК ПЕРЕДАЧИ
// ============================================================================
bool BSP_Uart_StartTx(BspUartId_t id) {
    if ((uint32_t)id >= BSP_UART_ID_COUNT) {
        return false;
    }

    BspUartCtx_t *ctx = &s_uart_ctx[id];
    if (ctx->tx_busy) {
        return false;
    }
    if (ctx->on_tx_next == 0) {
        return false;  // нечего передавать
    }

    ctx->tx_busy = true;
    if (ctx->is_rs485) {
        prv_SetRs485Dir(id, true);
    }

    LL_USART_ClearFlag_TC(ctx->instance);
    LL_USART_DisableIT_TC(ctx->instance);
    LL_USART_EnableIT_TXE(ctx->instance);
    return true;
}

bool BSP_Uart_IsTxBusy(BspUartId_t id) {
    if ((uint32_t)id >= BSP_UART_ID_COUNT) {
        return false;
    }
    return s_uart_ctx[id].tx_busy;
}

// ============================================================================
// ОБЩИЙ ОБРАБОТЧИК ПРЕРЫВАНИЙ (4 порта)
// ============================================================================
static void prv_Uart_IrqHandler(BspUartId_t id) {
    BspUartCtx_t *ctx = &s_uart_ctx[id];
    USART_TypeDef *uart = ctx->instance;

    // 1. ОБРАБОТКА ОШИБОК (ORE/FE/NE/PE) — ДО RXNE, чтобы не залипнуть
    //    Сброс: чтение SR (уже) + чтение DR
    if (LL_USART_IsActiveFlag_ORE(uart) != 0U || LL_USART_IsActiveFlag_FE(uart) != 0U ||
        LL_USART_IsActiveFlag_NE(uart) != 0U || LL_USART_IsActiveFlag_PE(uart) != 0U) {
        (void)LL_USART_ReceiveData8(uart);  // сброс ошибок
    }

    // 2. ПРИЁМ БАЙТА (RXNE)
    if (LL_USART_IsActiveFlag_RXNE(uart) != 0U) {
        uint8_t byte = LL_USART_ReceiveData8(uart);
        if (ctx->on_rx != 0) {
            ctx->on_rx(byte);
        }
    }

    // 3. IDLE — конец пакета (линия простаивает)
    if (LL_USART_IsActiveFlag_IDLE(uart) != 0U) {
        // Сброс IDLE: чтение SR + DR
        (void)LL_USART_ReceiveData8(uart);
        if (ctx->on_idle != 0) {
            ctx->on_idle();
        }
    }

    // 4. ПЕРЕДАЧА: TXE — запрашиваем следующий байт
    if ((LL_USART_IsEnabledIT_TXE(uart) != 0U) && (LL_USART_IsActiveFlag_TXE(uart) != 0U)) {
        uint8_t next_byte = 0U;
        if ((ctx->on_tx_next != 0) && ctx->on_tx_next(&next_byte)) {
            LL_USART_TransmitData8(uart, next_byte);
        } else {
            // Все байты загружены; ждём полного выхода стоп-бита (TC)
            LL_USART_DisableIT_TXE(uart);
            LL_USART_EnableIT_TC(uart);
        }
    }

    // 5. ФИЗИЧЕСКОЕ ЗАВЕРШЕНИЕ ПЕРЕДАЧИ (TC) — возвращаем RS-485 в приём
    if ((LL_USART_IsEnabledIT_TC(uart) != 0U) && (LL_USART_IsActiveFlag_TC(uart) != 0U)) {
        LL_USART_ClearFlag_TC(uart);
        LL_USART_DisableIT_TC(uart);
        if (ctx->is_rs485) {
            prv_SetRs485Dir(id, false);
        }
        ctx->tx_busy = false;
    }
}

void USART1_IRQHandler(void) {
    prv_Uart_IrqHandler(BSP_UART_ID_NEXTION);
}
void USART2_IRQHandler(void) {
    prv_Uart_IrqHandler(BSP_UART_ID_SILISTLAB);
}
void UART4_IRQHandler(void) {
    prv_Uart_IrqHandler(BSP_UART_ID_AUX_485);
}
void UART5_IRQHandler(void) {
    prv_Uart_IrqHandler(BSP_UART_ID_MODBUS);
}