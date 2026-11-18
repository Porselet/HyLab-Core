#ifndef CONF_HYLAB_M5A_H
#define CONF_HYLAB_M5A_H

// Паспорт серийной модели HyLab M5A (400 л/ч)
#define PRIBOR_CONFIG_SELECTED      1
#define PRIBOR_MODEL_ID             500
#define PRIBOR_MODEL_NAME           "HYLAB-M5A"

// Feature Flags (1 = есть, 0 = нет)
#define CFG_HAS_NEXTION_UI          1
#define CFG_HAS_H2_LEAK_SENSOR      1
#define CFG_HAS_EXT_PRESS_SENSOR    1
#define CFG_HAS_WATER_QUALITY       1
#define CFG_LEVEL_SENSORS_COUNT     8

// Силовая часть и пределы (по руководству HyLab M5A)
#define CFG_POWER_MODULES_COUNT     3
#define CFG_MAX_FLOW_ML_MIN         6600   // 400 л/ч
#define CFG_PRESS_WORK_SETPOINT_KPA 300    // 3.0 ати
#define CFG_PRESS_ALARM_LIMIT_KPA   400    // 4.0 ати
#define CFG_PRESS_CHECK_TARGET_KPA  100    // 1.0 ати за 2 мин
#define CFG_TEMP_MIN_START_C        5      // +5 °C мин. температура пуска
#define CFG_TEMP_ALARM_LIMIT_C      80     // +80 °C перегрев

#endif // CONF_HYLAB_M5A_H
