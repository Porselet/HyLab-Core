#include "project_config.h"
#include "bsp_init.h"
#include "scheduler.h"
#include "data_model.h"
#include "eeprom_storage.h"
#include "manager.h"

int main(void)
{
    // 0. Безопасное состояние всех выходов при старте
    // 1. Инициализация железа (BSP)
    // 2. Загрузка уставок и наработки из EEPROM
    // 3. Инициализация сервисов, менеджера и планировщика

    while (1)
    {
        // Scheduler_Run();
        // Сброс аппаратного Watchdog (IWDG)
    }
}
