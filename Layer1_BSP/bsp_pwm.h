#ifndef BSP_PWM_H
#define BSP_PWM_H

#include <stdint.h>

// ============================================================================
// АППАРАТНЫЙ ШИМ НА TIM4: 3 канала (насос, клапан перелива, клапан сброса)
// ============================================================================
// Инициализация доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
#ifdef BSP_INIT_INTERNAL
void BSP_PWM_Init(void);
#endif

// Управление скважностью (0..100%). Значения >100 клампятся в 100.
void BSP_Pwm_SetPump(uint8_t duty);
void BSP_Pwm_SetValvePour(uint8_t duty);
void BSP_Pwm_SetValveRelief(uint8_t duty);

// Аварийное отключение всех ШИМ-каналов (0%)
void BSP_Pwm_AllOff(void);

#endif  // BSP_PWM_H