#ifndef BSP_PINOUT_H
#define BSP_PINOUT_H

#ifndef STM32F103xE
#define STM32F103xE
#endif
#ifndef USE_FULL_LL_DRIVER
#define USE_FULL_LL_DRIVER
#endif

#include <stdbool.h>
#include <stdint.h>

#include "project_config.h"
#include "stm32f1xx.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"

// ============================================================================
// 1. ДИСКРЕТНЫЕ ВЫХОДЫ (GPIO OUTPUTS)
// ============================================================================

// Кондуктометр (полярность титановых электродов)
#define PIN_WQ_POL_A_PORT GPIOA
#define PIN_WQ_POL_A_PIN LL_GPIO_PIN_1

#define PIN_WQ_POL_B_PORT GPIOA
#define PIN_WQ_POL_B_PIN LL_GPIO_PIN_2

// Силовые ключи и питание периферии
#define PIN_COOLERS_PORT GPIOA
#define PIN_COOLERS_PIN LL_GPIO_PIN_7

#define PIN_NEXTION_EN_PORT GPIOA
#define PIN_NEXTION_EN_PIN LL_GPIO_PIN_11

#define PIN_PELTIER_PORT GPIOE  // D_OUT_4
#define PIN_PELTIER_PIN LL_GPIO_PIN_10

// Индикация и звук
#define PIN_POWER_LED_PORT GPIOC  // Светодиод на передней панели
#define PIN_POWER_LED_PIN LL_GPIO_PIN_9

#define PIN_LED1_PORT GPIOE
#define PIN_LED1_PIN LL_GPIO_PIN_2

#define PIN_LED2_PORT GPIOE
#define PIN_LED2_PIN LL_GPIO_PIN_3

#define PIN_LED3_PORT GPIOE
#define PIN_LED3_PIN LL_GPIO_PIN_4

#define PIN_BUZZER_PORT GPIOE
#define PIN_BUZZER_PIN LL_GPIO_PIN_9

// Управление направлением приемопередатчиков RS-485 (DE/RE)
#define PIN_RS485_DE2_PORT GPIOD  // USART2 (Silistlab)
#define PIN_RS485_DE2_PIN LL_GPIO_PIN_4

#define PIN_RS485_DE4_PORT GPIOA  // UART4 (Резервный RS-485)
#define PIN_RS485_DE4_PIN LL_GPIO_PIN_15

#define PIN_RS485_DE5_PORT GPIOD  // UART5 (Modbus Slave)
#define PIN_RS485_DE5_PIN LL_GPIO_PIN_1

// SPI1 Chip Select (резерв / внешняя память)
#define PIN_SPI1_CS_PORT GPIOB
#define PIN_SPI1_CS_PIN LL_GPIO_PIN_6

// ============================================================================
// 2. ШИМ-ВЫХОДЫ (TIM4 FULL REMAP: PD13, PD14, PD15)
// ============================================================================
#define PWM_TIM TIM4
#define PWM_PERIOD_TICKS 100U  // 0..100% (1 кГц при APB1_Tim=72 МГц, PSC=719)

#define PIN_DOUT1_PWM_PORT GPIOD  // PD13 -> TIM4_CH2 (D_OUT_1: Насос)
#define PIN_DOUT1_PWM_PIN LL_GPIO_PIN_13
#define PWM_CH_PUMP LL_TIM_CHANNEL_CH2

#define PIN_DOUT2_PWM_PORT GPIOD  // PD14 -> TIM4_CH3 (D_OUT_2: Клапан перелива)
#define PIN_DOUT2_PWM_PIN LL_GPIO_PIN_14
#define PWM_CH_VALVE_POUR LL_TIM_CHANNEL_CH3

#define PIN_DOUT3_PWM_PORT GPIOD  // PD15 -> TIM4_CH4 (D_OUT_3: Клапан сброса)
#define PIN_DOUT3_PWM_PIN LL_GPIO_PIN_15
#define PWM_CH_VALVE_RELIEF LL_TIM_CHANNEL_CH4

// ============================================================================
// 3. ДИСКРЕТНЫЕ ВХОДЫ (GPIO INPUTS & EXTI)
// ============================================================================
#define PIN_BTN_START_PORT GPIOA  // BTN_D_IN
#define PIN_BTN_START_PIN LL_GPIO_PIN_8

#define PIN_DIN1_PORT GPIOE  // D_IN_1
#define PIN_DIN1_PIN LL_GPIO_PIN_13

#define PIN_DIN2_PORT GPIOE  // D_IN_2
#define PIN_DIN2_PIN LL_GPIO_PIN_14

#define PIN_DIN3_PORT GPIOE  // D_IN_3
#define PIN_DIN3_PIN LL_GPIO_PIN_15

#define PIN_DIN4_PORT GPIOB  // D_IN_4
#define PIN_DIN4_PIN LL_GPIO_PIN_10

#define PIN_DIN5_PORT GPIOB  // D_IN_5
#define PIN_DIN5_PIN LL_GPIO_PIN_11

#define PIN_DIN6_PORT GPIOB  // D_IN_6
#define PIN_DIN6_PIN LL_GPIO_PIN_12

#define PIN_DIN7_PORT GPIOB  // D_IN_7
#define PIN_DIN7_PIN LL_GPIO_PIN_13

#define PIN_VSENSE_PORT GPIOC  // V_SENSE (EXTI3 по спаду)
#define PIN_VSENSE_PIN LL_GPIO_PIN_3

#define PIN_SD_DETECT_PORT GPIOD  // CD_CARD_DETECT
#define PIN_SD_DETECT_PIN LL_GPIO_PIN_7

// ============================================================================
// 4. АНАЛОГОВЫЕ ВХОДЫ (ADC1 - 11 КАНАЛОВ В ПОРЯДКЕ РАНГОВ DMA)
// ============================================================================
// ============================================================================
// АЦП1 + DMA1: 4 канала, пачка по 300 циклов (normal mode)
// ============================================================================
// Порядок каналов в последовательности (rank 1..4):
//   [0] WQ_VALUE      (PA0 / IN0)
//   [1] PRESS_4_20    (PC1 / IN11)
//   [2] PRESS_MAP     (PC0 / IN10)
//   [3] TEMP_1        (PC2 / IN12)
//
// В буфере DMA: 1200 отсчётов (300 циклов × 4 канала).
// WQ-отсчёты: buf[0], buf[4], buf[8], ... (шаг 4).
// ============================================================================

#define BSP_ADC_CHANNELS_COUNT 4U
#define BSP_ADC_BATCH_CYCLES 300U
#define BSP_ADC_BATCH_SAMPLES (BSP_ADC_BATCH_CYCLES * BSP_ADC_CHANNELS_COUNT)

// Индексы каналов в последовательности
typedef enum {
    ADC_IDX_WQ = 0,      // Rank 1: PA0 -> IN0
    ADC_IDX_PRESS_4_20,  // Rank 2: PC1 -> IN11
    ADC_IDX_PRESS_MAP,   // Rank 3: PC0 -> IN10
    ADC_IDX_TEMP_1       // Rank 4: PC2 -> IN12
} BspAdcChannelIdx_t;

// ============================================================================
// 5. ПОСЛЕДОВАТЕЛЬНЫЕ ПОРТЫ И I2C (ПЕРИФЕРИЯ И НОЖКИ)
// ============================================================================
// USART1: Nextion UI (PA9 TX, PA10 RX)
#define BSP_UART_NEXTION USART1
#define PIN_USART1_TX_PORT GPIOA
#define PIN_USART1_TX_PIN LL_GPIO_PIN_9
#define PIN_USART1_RX_PORT GPIOA
#define PIN_USART1_RX_PIN LL_GPIO_PIN_10

// USART2 (Full Remap): Silistlab RS-485 (PD5 TX, PD6 RX, PD4 DE)
#define BSP_UART_SILISTLAB USART2
#define PIN_USART2_TX_PORT GPIOD
#define PIN_USART2_TX_PIN LL_GPIO_PIN_5
#define PIN_USART2_RX_PORT GPIOD
#define PIN_USART2_RX_PIN LL_GPIO_PIN_6

// UART4: Резервный RS-485 (PC10 TX, PC11 RX, PA15 DE)
#define BSP_UART_AUX UART4
#define PIN_UART4_TX_PORT GPIOC
#define PIN_UART4_TX_PIN LL_GPIO_PIN_10
#define PIN_UART4_RX_PORT GPIOC
#define PIN_UART4_RX_PIN LL_GPIO_PIN_11

// UART5: Modbus RTU Slave (PC12 TX, PD2 RX, PD1 DE)
#define BSP_UART_MODBUS UART5
#define PIN_UART5_TX_PORT GPIOC
#define PIN_UART5_TX_PIN LL_GPIO_PIN_12
#define PIN_UART5_RX_PORT GPIOD
#define PIN_UART5_RX_PIN LL_GPIO_PIN_2

// I2C1 (Remap): RTC DS3231 (PB8 SCL, PB9 SDA)
#define BSP_I2C_RTC I2C1
#define PIN_I2C1_SCL_PORT GPIOB
#define PIN_I2C1_SCL_PIN LL_GPIO_PIN_8
#define PIN_I2C1_SDA_PORT GPIOB
#define PIN_I2C1_SDA_PIN LL_GPIO_PIN_9

// ============================================================================
// 6. АТОМАРНЫЕ INLINE-ФУНКЦИИ УПРАВЛЕНИЯ ВЫХОДАМИ И ЧТЕНИЯ ВХОДОВ
//    (Единственный интерфейс для Layer3_Services/io_devices.c)
// ============================================================================

static inline void BSP_Pin_Set(GPIO_TypeDef *port, uint32_t pin, bool state) {
    if (state) {
        LL_GPIO_SetOutputPin(port, pin);
    } else {
        LL_GPIO_ResetOutputPin(port, pin);
    }
}

static inline bool BSP_Pin_Read(GPIO_TypeDef *port, uint32_t pin) {
    return (LL_GPIO_IsInputPinSet(port, pin) != 0U);
}

// Управление дискретными выходами
static inline void BSP_Out_WqPolA(bool on) {
    BSP_Pin_Set(PIN_WQ_POL_A_PORT, PIN_WQ_POL_A_PIN, on);
}
static inline void BSP_Out_WqPolB(bool on) {
    BSP_Pin_Set(PIN_WQ_POL_B_PORT, PIN_WQ_POL_B_PIN, on);
}
static inline void BSP_Out_Coolers(bool on) {
    BSP_Pin_Set(PIN_COOLERS_PORT, PIN_COOLERS_PIN, on);
}
static inline void BSP_Out_NextionEn(bool on) {
    BSP_Pin_Set(PIN_NEXTION_EN_PORT, PIN_NEXTION_EN_PIN, on);
}
static inline void BSP_Out_Peltier(bool on) {
    BSP_Pin_Set(PIN_PELTIER_PORT, PIN_PELTIER_PIN, on);
}
static inline void BSP_Out_PowerLed(bool on) {
    BSP_Pin_Set(PIN_POWER_LED_PORT, PIN_POWER_LED_PIN, on);
}
static inline void BSP_Out_Led1(bool on) {
    BSP_Pin_Set(PIN_LED1_PORT, PIN_LED1_PIN, on);
}
static inline void BSP_Out_Led2(bool on) {
    BSP_Pin_Set(PIN_LED2_PORT, PIN_LED2_PIN, on);
}
static inline void BSP_Out_Led3(bool on) {
    BSP_Pin_Set(PIN_LED3_PORT, PIN_LED3_PIN, on);
}
static inline void BSP_Out_Buzzer(bool on) {
    BSP_Pin_Set(PIN_BUZZER_PORT, PIN_BUZZER_PIN, on);
}

// Управление направлениями RS-485
static inline void BSP_RS485_SetTx2(bool tx_en) {
    BSP_Pin_Set(PIN_RS485_DE2_PORT, PIN_RS485_DE2_PIN, tx_en);
}
static inline void BSP_RS485_SetTx4(bool tx_en) {
    BSP_Pin_Set(PIN_RS485_DE4_PORT, PIN_RS485_DE4_PIN, tx_en);
}
static inline void BSP_RS485_SetTx5(bool tx_en) {
    BSP_Pin_Set(PIN_RS485_DE5_PORT, PIN_RS485_DE5_PIN, tx_en);
}

// Чтение дискретных входов (сырой логический уровень на ножке)
static inline bool BSP_In_BtnStart(void) {
    return BSP_Pin_Read(PIN_BTN_START_PORT, PIN_BTN_START_PIN);
}
static inline bool BSP_In_Din1(void) {
    return BSP_Pin_Read(PIN_DIN1_PORT, PIN_DIN1_PIN);
}
static inline bool BSP_In_Din2(void) {
    return BSP_Pin_Read(PIN_DIN2_PORT, PIN_DIN2_PIN);
}
static inline bool BSP_In_Din3(void) {
    return BSP_Pin_Read(PIN_DIN3_PORT, PIN_DIN3_PIN);
}
static inline bool BSP_In_Din4(void) {
    return BSP_Pin_Read(PIN_DIN4_PORT, PIN_DIN4_PIN);
}
static inline bool BSP_In_Din5(void) {
    return BSP_Pin_Read(PIN_DIN5_PORT, PIN_DIN5_PIN);
}
static inline bool BSP_In_Din6(void) {
    return BSP_Pin_Read(PIN_DIN6_PORT, PIN_DIN6_PIN);
}
static inline bool BSP_In_Din7(void) {
    return BSP_Pin_Read(PIN_DIN7_PORT, PIN_DIN7_PIN);
}
static inline bool BSP_In_VSense(void) {
    return BSP_Pin_Read(PIN_VSENSE_PORT, PIN_VSENSE_PIN);
}
static inline bool BSP_In_SdDetect(void) {
    return BSP_Pin_Read(PIN_SD_DETECT_PORT, PIN_SD_DETECT_PIN);
}

#endif  // BSP_PINOUT_H