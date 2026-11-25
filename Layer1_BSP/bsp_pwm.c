#define BSP_INIT_INTERNAL
#include "bsp_pwm.h"

#include "bsp_pinout.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ TIM4: 3-канальный ШИМ, 1 кГц, 0..100%
// ============================================================================
#ifdef BSP_INIT_INTERNAL
void BSP_PWM_Init(void) {
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

    // Перевод PD13, PD14, PD15 в Alternate Function Push-Pull
    LL_GPIO_InitTypeDef gpio = {0};
    gpio.Pin = PIN_DOUT1_PWM_PIN | PIN_DOUT2_PWM_PIN | PIN_DOUT3_PWM_PIN;
    gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOD, &gpio);

    // Настройка базы таймера:
    // Тактирование TIM4 = 72 МГц.
    // PSC = 719 -> частота счёта 100 кГц (период тика 10 мкс).
    // ARR = 99  -> период ШИМ 100 тиков = 1000 мкс (1 кГц), заполнение 0..100%.
    LL_TIM_InitTypeDef tim = {0};
    tim.Prescaler = 719U;
    tim.CounterMode = LL_TIM_COUNTERMODE_UP;
    tim.Autoreload = PWM_PERIOD_TICKS - 1U;
    tim.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(PWM_TIM, &tim);
    LL_TIM_EnableARRPreload(PWM_TIM);

    // Настройка каналов CH2, CH3, CH4 в режиме PWM Mode 1 (начальное заполнение = 0)
    LL_TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = LL_TIM_OCMODE_PWM1;
    oc.OCState = LL_TIM_OCSTATE_ENABLE;
    oc.CompareValue = 0U;
    oc.OCPolarity = LL_TIM_OCPOLARITY_HIGH;

    LL_TIM_OC_Init(PWM_TIM, PWM_CH_PUMP, &oc);
    LL_TIM_OC_EnablePreload(PWM_TIM, PWM_CH_PUMP);

    LL_TIM_OC_Init(PWM_TIM, PWM_CH_VALVE_POUR, &oc);
    LL_TIM_OC_EnablePreload(PWM_TIM, PWM_CH_VALVE_POUR);

    LL_TIM_OC_Init(PWM_TIM, PWM_CH_VALVE_RELIEF, &oc);
    LL_TIM_OC_EnablePreload(PWM_TIM, PWM_CH_VALVE_RELIEF);

    LL_TIM_GenerateEvent_UPDATE(PWM_TIM);
    LL_TIM_EnableCounter(PWM_TIM);
}
#endif  // BSP_INIT_INTERNAL

// ============================================================================
// УПРАВЛЕНИЕ СКВАЖНОСТЬЮ (0..100%)
// ============================================================================
void BSP_Pwm_SetPump(uint8_t duty) {
    if (duty > 100U) {
        duty = 100U;
    }
    LL_TIM_OC_SetCompareCH2(PWM_TIM, duty);
}

void BSP_Pwm_SetValvePour(uint8_t duty) {
    if (duty > 100U) {
        duty = 100U;
    }
    LL_TIM_OC_SetCompareCH3(PWM_TIM, duty);
}

void BSP_Pwm_SetValveRelief(uint8_t duty) {
    if (duty > 100U) {
        duty = 100U;
    }
    LL_TIM_OC_SetCompareCH4(PWM_TIM, duty);
}

// ============================================================================
// АВАРИЙНОЕ ОТКЛЮЧЕНИЕ ВСЕХ КАНАЛОВ
// ============================================================================
void BSP_Pwm_AllOff(void) {
    LL_TIM_OC_SetCompareCH2(PWM_TIM, 0U);
    LL_TIM_OC_SetCompareCH3(PWM_TIM, 0U);
    LL_TIM_OC_SetCompareCH4(PWM_TIM, 0U);
}