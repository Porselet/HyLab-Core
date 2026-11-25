#define BSP_INIT_INTERNAL
#include "bsp_gpio.h"

#include "bsp_pinout.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"

// ============================================================================
// НАСТРОЙКА ВХОДОВ, АНАЛОГОВЫХ ПОРТОВ И РЕМАПОВ ПЕРИФЕРИИ
// ============================================================================
#ifdef BSP_INIT_INTERNAL
void BSP_GPIO_Init(void) {
    LL_GPIO_InitTypeDef gpio = {0};

    // --- Аппаратные ремапы платы RedBoardSmall V2.0 ---
    LL_GPIO_AF_EnableRemap_USART2();  // PD5 (TX), PD6 (RX)
    LL_GPIO_AF_EnableRemap_TIM4();    // PD13 (CH2), PD14 (CH3), PD15 (CH4)
    LL_GPIO_AF_EnableRemap_I2C1();    // PB8 (SCL), PB9 (SDA)

    // --- 1. Дискретные входы (Floating Input, внешняя подтяжка на плате) ---
    gpio.Mode = LL_GPIO_MODE_FLOATING;

    gpio.Pin = PIN_BTN_START_PIN;
    LL_GPIO_Init(PIN_BTN_START_PORT, &gpio);

    gpio.Pin = PIN_DIN1_PIN | PIN_DIN2_PIN | PIN_DIN3_PIN;
    LL_GPIO_Init(GPIOE, &gpio);

    gpio.Pin = PIN_DIN4_PIN | PIN_DIN5_PIN | PIN_DIN6_PIN | PIN_DIN7_PIN;
    LL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = PIN_VSENSE_PIN;
    LL_GPIO_Init(PIN_VSENSE_PORT, &gpio);

    gpio.Pin = PIN_SD_DETECT_PIN;
    LL_GPIO_Init(PIN_SD_DETECT_PORT, &gpio);

    // --- 2. Аналоговые входы для ADC1 (11 каналов) ---
    gpio.Mode = LL_GPIO_MODE_ANALOG;

    // PA0 (IN0), PA4 (IN4), PA5 (IN5), PA6 (IN6)
    gpio.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6;
    LL_GPIO_Init(GPIOA, &gpio);

    // PB0 (IN8), PB1 (IN9)
    gpio.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1;
    LL_GPIO_Init(GPIOB, &gpio);

    // PC0 (IN10), PC1 (IN11), PC2 (IN12), PC4 (IN14), PC5 (IN15)
    gpio.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOC, &gpio);
}
#endif  // BSP_INIT_INTERNAL