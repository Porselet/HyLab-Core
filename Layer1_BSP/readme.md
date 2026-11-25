# BSP Layer — Facade API

Слой BSP для платы RedBoardSmall V2.0 (STM32F103VET6, 72 МГц, Cortex-M3).
Все init-функции скрыты за BSP_INIT_INTERNAL — видны только из bsp_init.c.
Верхние слои работают через фасад bsp_init.h и публичные API модулей.

---

## 1. Инициализация (фасад)

    #include "bsp_init.h"

    BSP_SafeOutputsInit();   // Вызывается ПЕРВОЙ строкой main() — гасит все выходы
    BSP_SystemInit();        // Полная инициализация: RCC, GPIO, PWM, SysTick, UART, ADC, I2C, IWDG

BSP_SystemInit() вызывает по порядку:
SafeOutputs → Clock → GPIO → PWM → Tick → UART×4 → ADC → I2C → Watchdog(500 мс)

---

## 2. Системное время и сторож (bsp_init.h реэкспортирует)

    uint32_t BSP_GetTickMs(void);       // Время с момента старта (мс), монотонно
    void     BSP_DelayMs(uint32_t ms);  // Блокирующая задержка (ТОЛЬКО для init!)
    void     BSP_Watchdog_Feed(void);   // Сброс IWDG — в конце while(1)

Правило: BSP_Watchdog_Feed() — один раз в конце главного цикла, не в прерываниях.

---

## 3. GPIO — входы/выходы (bsp_pinout.h, inline)

Дискретные выходы:

    BSP_Out_WqPolA(bool on);      // PA1  — кондуктометр, полярность A
    BSP_Out_WqPolB(bool on);      // PA2  — кондуктометр, полярность B
    BSP_Out_Coolers(bool on);     // PA7  — вентиляторы
    BSP_Out_NextionEn(bool on);   // PA11 — питание Nextion
    BSP_Out_Peltier(bool on);     // PE10 — Пельтье
    BSP_Out_PowerLed(bool on);    // PC9  — светодиод питания
    BSP_Out_Led1/2/3(bool on);    // PE2/3/4 — индикация
    BSP_Out_Buzzer(bool on);      // PE9  — зуммер

Дискретные входы:

    BSP_In_BtnStart(void);   // PA8
    BSP_In_Din1..7(void);    // PE13/14/15, PB10/11/12/13
    BSP_In_VSense(void);     // PC3
    BSP_In_SdDetect(void);   // PD7

RS-485 направление (DE):

    BSP_RS485_SetTx2(bool tx);  // PD4 — Silistlab
    BSP_RS485_SetTx4(bool tx);  // PA15 — AUX
    BSP_RS485_SetTx5(bool tx);  // PD1 — Modbus

Универсальные:

    BSP_Pin_Set(GPIO_TypeDef *port, uint32_t pin, bool state);
    BSP_Pin_Read(GPIO_TypeDef *port, uint32_t pin);

---

## 4. ШИМ — TIM4, 3 канала (bsp_pwm.h)

    BSP_Pwm_SetPump(uint8_t duty);        // PD13 / CH2 — насос (0..100%)
    BSP_Pwm_SetValvePour(uint8_t duty);   // PD14 / CH3 — клапан перелива
    BSP_Pwm_SetValveRelief(uint8_t duty); // PD15 / CH4 — клапан сброса
    BSP_Pwm_AllOff(void);                 // Аварийное обнуление всех каналов

Частота 1 кГц, скважность 0..100% (значения >100 клампятся).

---

## 5. UART — 4 порта (bsp_uart.h)

    typedef enum {
        BSP_UART_ID_NEXTION,    // USART1: PA9/PA10, 9600 8N1
        BSP_UART_ID_SILISTLAB,  // USART2: PD5/PD6 + PD4 DE, 19200 9E1
        BSP_UART_ID_AUX_485,    // UART4:  PC10/PC11 + PA15 DE, 9600 8N1
        BSP_UART_ID_MODBUS,     // UART5:  PC12/PD2 + PD1 DE, 9600 8N1
        BSP_UART_ID_COUNT
    } BspUartId_t;

Коллбэки (регистрируются из Layer2):

    void BSP_Uart_SetCallbacks(
        BspUartId_t id,
        void (*on_rx)(uint8_t byte),           // Приём байта (из ISR)
        bool (*on_tx_next)(uint8_t *out),      // Запрос следующего байта (из ISR)
        void (*on_idle)(void)                  // Конец пакета по IDLE (из ISR)
    );

Управление передачей:

    bool BSP_Uart_StartTx(BspUartId_t id);   // Запуск передачи (false если занят/нет on_tx_next)
    bool BSP_Uart_IsTxBusy(BspUartId_t id);  // Занят ли передатчик

RS-485 DE поднимается в StartTx, опускается по флагу TC (после физического выхода стоп-бита).

---

## 6. ADC — 4 канала, batch (bsp_adc.h)

    #define BSP_ADC_CHANNELS_COUNT   4U
    #define BSP_ADC_BATCH_CYCLES     300U
    #define BSP_ADC_BATCH_SAMPLES    (BSP_ADC_BATCH_CYCLES * BSP_ADC_CHANNELS_COUNT)  // 1200

    typedef enum {
        ADC_IDX_WQ = 0,       // PA0 / IN0   — кондуктометр
        ADC_IDX_PRESS_4_20,   // PC1 / IN11  — давление 4-20 мА
        ADC_IDX_PRESS_MAP,    // PC0 / IN10  — давление Motorola
        ADC_IDX_TEMP_1        // PC2 / IN12  — температура NTC
    } BspAdcChannelIdx_t;

API:

    void BSP_ADC_StartBatch(void);              // Запуск пачки 300 циклов × 4 канала
    bool BSP_ADC_IsBatchDone(void);             // Проверка завершения (сбрасывает флаг)
    const uint16_t *BSP_ADC_GetBatchBuffer(void); // Указатель на 1200 отсчётов
    void BSP_ADC_Stop(void);                    // Аварийная остановка

Формат буфера: [WQ, P4, PM, T] × 300. WQ на индексах 0, 4, 8, ...

Параметры: время выборки 239.5 циклов, continuous mode, DMA normal, 1200 отсчётов,
остановка ADC в ISR DMA.

---

## 7. I2C — RTC DS3231 (bsp_i2c.h)

    bool BSP_I2C_IsDeviceReady(uint8_t dev_addr_7bit);
    bool BSP_I2C_WriteReg(uint8_t dev_addr_7bit, uint8_t reg_addr,
                          const uint8_t *data, uint16_t len);
    bool BSP_I2C_ReadReg(uint8_t dev_addr_7bit, uint8_t reg_addr,
                         uint8_t *data, uint16_t len);

I2C1: PB8 (SCL), PB9 (SDA), 100 кГц, 7-битная адресация. BSP_I2C_RTC = I2C1.

---

## 8. Flash — EEPROM-эмуляция (bsp_flash.h)

    #define EEPROM_PAGE_0_ADDR   0x0807F000UL   // 2 КБ
    #define EEPROM_PAGE_1_ADDR   0x0807F800UL   // 2 КБ
    #define EEPROM_PAGE_SIZE     2048UL

    void BSP_Flash_Unlock(void);
    void BSP_Flash_Lock(void);
    bool BSP_Flash_ErasePage(uint32_t page_addr);
    bool BSP_Flash_WriteHalfWord(uint32_t addr, uint16_t data);
    bool BSP_Flash_WriteWord32(uint32_t addr, uint32_t data);

Минимальная единица записи — 16 бит. Стирание — только целой страницей (2 КБ).
Внутри ожидания BSY вызывается BSP_Watchdog_Feed().

---

## 9. Что скрыто за BSP_INIT_INTERNAL

Эти функции видны только из bsp_init.c:

    BSP_Clock_Init72MHz();
    BSP_GPIO_Init();
    BSP_PWM_Init();
    BSP_Tick_Init();
    BSP_Watchdog_Start(uint32_t timeout_ms);
    BSP_Uart_Init(BspUartId_t id, uint32_t baudrate);
    BSP_ADC_Init();
    BSP_I2C_Init();

Верхние слои (Layer2/3/4) их не видят — попытка вызова даст implicit declaration.

---

## 10. Запреты (project_config.h)

    malloc()  → _Static_assert(0, "...")   // Запрещено
    calloc()  → _Static_assert(0, "...")   // Запрещено
    free()    → _Static_assert(0, "...")   // Запрещено

Динамическая память полностью исключена. Только статическое выделение.