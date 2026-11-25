// Единственный файл, которому разрешено видеть init-функции модулей BSP
#define BSP_INIT_INTERNAL

#include "bsp_init.h"

#include "bsp_adc.h"
#include "bsp_clock.h"
#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include "bsp_pinout.h"
#include "bsp_pwm.h"
#include "bsp_tick.h"
#include "bsp_uart.h"
#include "bsp_watchdog.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_system.h"

// SystemCoreClock определён в bsp_clock.c

// ============================================================================
// БЕЗОПАСНАЯ ИНИЦИАЛИЗАЦИЯ СИЛОВЫХ ВЫХОДОВ (до настройки тактирования)
// ============================================================================
void BSP_SafeOutputsInit(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA | LL_APB2_GRP1_PERIPH_GPIOB | LL_APB2_GRP1_PERIPH_GPIOC |
                             LL_APB2_GRP1_PERIPH_GPIOD | LL_APB2_GRP1_PERIPH_GPIOE | LL_APB2_GRP1_PERIPH_AFIO);

    // Освобождаем PA15 (JTAG отключен, SWD на PA13/PA14 остаётся активным)
    LL_GPIO_AF_Remap_SWJ_NOJTAG();

    // 1. Выставляем безопасные уровни в ODR ДО перевода пинов в режим Output
    LL_GPIO_ResetOutputPin(PIN_WQ_POL_A_PORT, PIN_WQ_POL_A_PIN);
    LL_GPIO_ResetOutputPin(PIN_WQ_POL_B_PORT, PIN_WQ_POL_B_PIN);
    LL_GPIO_ResetOutputPin(PIN_COOLERS_PORT, PIN_COOLERS_PIN);
    LL_GPIO_ResetOutputPin(PIN_NEXTION_EN_PORT, PIN_NEXTION_EN_PIN);
    LL_GPIO_ResetOutputPin(PIN_PELTIER_PORT, PIN_PELTIER_PIN);
    LL_GPIO_ResetOutputPin(PIN_POWER_LED_PORT, PIN_POWER_LED_PIN);
    LL_GPIO_ResetOutputPin(PIN_LED1_PORT, PIN_LED1_PIN);
    LL_GPIO_ResetOutputPin(PIN_LED2_PORT, PIN_LED2_PIN);
    LL_GPIO_ResetOutputPin(PIN_LED3_PORT, PIN_LED3_PIN);
    LL_GPIO_ResetOutputPin(PIN_BUZZER_PORT, PIN_BUZZER_PIN);

    // ШИМ-выходы D_OUT_1..3 в низкий уровень
    LL_GPIO_ResetOutputPin(PIN_DOUT1_PWM_PORT, PIN_DOUT1_PWM_PIN);
    LL_GPIO_ResetOutputPin(PIN_DOUT2_PWM_PORT, PIN_DOUT2_PWM_PIN);
    LL_GPIO_ResetOutputPin(PIN_DOUT3_PWM_PORT, PIN_DOUT3_PWM_PIN);

    // RS-485 приемопередатчики переводим в режим приёма (DE = 0)
    LL_GPIO_ResetOutputPin(PIN_RS485_DE2_PORT, PIN_RS485_DE2_PIN);
    LL_GPIO_ResetOutputPin(PIN_RS485_DE4_PORT, PIN_RS485_DE4_PIN);
    LL_GPIO_ResetOutputPin(PIN_RS485_DE5_PORT, PIN_RS485_DE5_PIN);

    // SPI1 CS в неактивное состояние (High)
    LL_GPIO_SetOutputPin(PIN_SPI1_CS_PORT, PIN_SPI1_CS_PIN);

    // 2. Переводим все управляющие пины в Push-Pull Output
    LL_GPIO_InitTypeDef gpio = {0};
    gpio.Mode = LL_GPIO_MODE_OUTPUT;
    gpio.Speed = LL_GPIO_SPEED_FREQ_LOW;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;

    // Port A
    gpio.Pin = PIN_WQ_POL_A_PIN | PIN_WQ_POL_B_PIN | PIN_COOLERS_PIN | PIN_NEXTION_EN_PIN | PIN_RS485_DE4_PIN;
    LL_GPIO_Init(GPIOA, &gpio);

    // Port B
    gpio.Pin = PIN_SPI1_CS_PIN;
    LL_GPIO_Init(GPIOB, &gpio);

    // Port C
    gpio.Pin = PIN_POWER_LED_PIN;
    LL_GPIO_Init(GPIOC, &gpio);

    // Port D
    gpio.Pin = PIN_RS485_DE2_PIN | PIN_RS485_DE5_PIN | PIN_DOUT1_PWM_PIN | PIN_DOUT2_PWM_PIN | PIN_DOUT3_PWM_PIN;
    LL_GPIO_Init(GPIOD, &gpio);

    // Port E
    gpio.Pin = PIN_PELTIER_PIN | PIN_LED1_PIN | PIN_LED2_PIN | PIN_LED3_PIN | PIN_BUZZER_PIN;
    LL_GPIO_Init(GPIOE, &gpio);
}

// ============================================================================
// ПОЛНАЯ СИСТЕМНАЯ ИНИЦИАЛИЗАЦИЯ (порядок критичен!)
// ============================================================================
void BSP_SystemInit(void) {
    // 1. Группировка приоритетов NVIC (4 бита на вытеснение, 0 на подприоритет)
    NVIC_SetPriorityGrouping(0x00000003U);

    // 2. Гарантируем безопасное состояние выходов
    BSP_SafeOutputsInit();

    // 3. Тактирование 72 МГц от внешнего кварца 16 МГц
    BSP_Clock_Init72MHz();

    // 4. GPIO: входы, аналоговые пины, ремапы периферии
    BSP_GPIO_Init();

    // 5. Аппаратный ШИМ на TIM4 (1 кГц, 0%)
    BSP_PWM_Init();

    // 6. SysTick на 1 мс
    BSP_Tick_Init();

    // 7. UART-порты (Nextion, Silistlab, AUX, Modbus)
    BSP_Uart_Init(BSP_UART_ID_NEXTION, 9600);
    BSP_Uart_Init(BSP_UART_ID_SILISTLAB, 19200);
    BSP_Uart_Init(BSP_UART_ID_AUX_485, 9600);
    BSP_Uart_Init(BSP_UART_ID_MODBUS, 9600);

    // 8. ADC1 + DMA (11 каналов)
    BSP_ADC_Init();

    // 9. I2C1 (RTC DS3231)
    BSP_I2C_Init();

    // 10. Аппаратный сторожевой таймер (~500 мс) — ПОСЛЕДНИМ
    BSP_Watchdog_Start(500);
}