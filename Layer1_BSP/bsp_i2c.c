#define BSP_INIT_INTERNAL
#include "bsp_i2c.h"

#include "bsp_init.h"
#include "bsp_pinout.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_i2c.h"

// Таймаут одной фазы транзакции (мс)
#define I2C_TIMEOUT_MS 5U

// ============================================================================
// ВНУТРЕННИЕ ПРОТОТИПЫ
// ============================================================================
static void prv_BusRecover(void);
static bool prv_WaitFlagSet(uint32_t flag);
static bool prv_WaitFlagReset(uint32_t flag);
static void prv_ClearAddrFlag(void);

// ============================================================================
// РАЗБЛОКИРОВКА ЗАВИСШЕЙ ШИНЫ (9 тактов SCL + STOP)
// ============================================================================
static void prv_BusRecover(void) {
    // Переводим SCL и SDA в Open-Drain Output
    LL_GPIO_InitTypeDef gpio = {0};
    gpio.Mode = LL_GPIO_MODE_OUTPUT;
    gpio.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    gpio.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;

    gpio.Pin = PIN_I2C1_SCL_PIN;
    LL_GPIO_Init(PIN_I2C1_SCL_PORT, &gpio);

    gpio.Pin = PIN_I2C1_SDA_PIN;
    LL_GPIO_Init(PIN_I2C1_SDA_PORT, &gpio);

    // Отпускаем обе линии (High)
    LL_GPIO_SetOutputPin(PIN_I2C1_SCL_PORT, PIN_I2C1_SCL_PIN);
    LL_GPIO_SetOutputPin(PIN_I2C1_SDA_PORT, PIN_I2C1_SDA_PIN);

    // Если SDA притянута к земле — тактируем SCL до освобождения (макс. 9 тактов)
    for (uint8_t i = 0U; i < 9U; i++) {
        if (LL_GPIO_IsInputPinSet(PIN_I2C1_SDA_PORT, PIN_I2C1_SDA_PIN) != 0U) {
            break;
        }
        LL_GPIO_ResetOutputPin(PIN_I2C1_SCL_PORT, PIN_I2C1_SCL_PIN);
        for (volatile uint32_t d = 0U; d < 100U; d++) {
            __NOP();
        }
        LL_GPIO_SetOutputPin(PIN_I2C1_SCL_PORT, PIN_I2C1_SCL_PIN);
        for (volatile uint32_t d = 0U; d < 100U; d++) {
            __NOP();
        }
    }

    // Формируем STOP: SDA 0->1 при SCL=1
    LL_GPIO_ResetOutputPin(PIN_I2C1_SDA_PORT, PIN_I2C1_SDA_PIN);
    for (volatile uint32_t d = 0U; d < 100U; d++) {
        __NOP();
    }
    LL_GPIO_SetOutputPin(PIN_I2C1_SCL_PORT, PIN_I2C1_SCL_PIN);
    for (volatile uint32_t d = 0U; d < 100U; d++) {
        __NOP();
    }
    LL_GPIO_SetOutputPin(PIN_I2C1_SDA_PORT, PIN_I2C1_SDA_PIN);
}

// ============================================================================
// ОЖИДАНИЕ ФЛАГОВ С ТАЙМАУТОМ
// ============================================================================
static bool prv_WaitFlagSet(uint32_t flag) {
    uint32_t start = BSP_GetTickMs();
    while ((BSP_I2C_RTC->SR1 & flag) == 0U) {
        if ((BSP_GetTickMs() - start) >= I2C_TIMEOUT_MS) {
            return false;
        }
        // Если взвёлся NACK (AF) — прерываем ожидание
        if ((BSP_I2C_RTC->SR1 & I2C_SR1_AF) != 0U) {
            BSP_I2C_RTC->SR1 &= ~I2C_SR1_AF;
            return false;
        }
    }
    return true;
}

static bool prv_WaitFlagReset(uint32_t flag) {
    uint32_t start = BSP_GetTickMs();
    while ((BSP_I2C_RTC->SR1 & flag) != 0U) {
        if ((BSP_GetTickMs() - start) >= I2C_TIMEOUT_MS) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// СБРОС ФЛАГА ADDR (чтение SR1 + SR2)
// ============================================================================
static void prv_ClearAddrFlag(void) {
    (void)BSP_I2C_RTC->SR1;
    (void)BSP_I2C_RTC->SR2;
}

// ============================================================================
// ИНИЦИАЛИЗАЦИЯ I2C1
// ============================================================================
#ifdef BSP_INIT_INTERNAL
void BSP_I2C_Init(void) {
    // 1. Тактирование GPIOB и I2C1
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    // 2. Разблокировка шины (до настройки I2C)
    prv_BusRecover();

    // 3. Настройка пинов PB8/PB9 в AF Open-Drain
    LL_GPIO_InitTypeDef gpio = {0};
    gpio.Pin = PIN_I2C1_SCL_PIN | PIN_I2C1_SDA_PIN;
    gpio.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(GPIOB, &gpio);

    // 4. Программный сброс блока I2C1 (снятие зависшего BUSY)
    LL_I2C_EnableReset(BSP_I2C_RTC);
    LL_I2C_DisableReset(BSP_I2C_RTC);

    // 5. Конфигурация I2C1 на 100 кГц
    LL_I2C_InitTypeDef i2c = {0};
    i2c.PeripheralMode = LL_I2C_MODE_I2C;
    i2c.ClockSpeed = 100000U;
    i2c.DutyCycle = LL_I2C_DUTYCYCLE_2;
    i2c.OwnAddress1 = 0U;
    i2c.TypeAcknowledge = LL_I2C_ACK;
    i2c.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(BSP_I2C_RTC, &i2c);

    LL_I2C_Enable(BSP_I2C_RTC);
}
#endif  // BSP_INIT_INTERNAL

// ============================================================================
// ПРОВЕРКА ГОТОВНОСТИ УСТРОЙСТВА
// ============================================================================
bool BSP_I2C_IsDeviceReady(uint8_t dev_addr_7bit) {
    if (!prv_WaitFlagReset(I2C_SR2_BUSY)) {
        return false;
    }

    BSP_I2C_RTC->SR1 &= ~I2C_SR1_AF;

    // START
    LL_I2C_GenerateStartCondition(BSP_I2C_RTC);
    if (!prv_WaitFlagSet(I2C_SR1_SB)) {
        LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
        return false;
    }

    // Адрес (Write)
    LL_I2C_TransmitData8(BSP_I2C_RTC, (uint8_t)(dev_addr_7bit << 1U));
    if (!prv_WaitFlagSet(I2C_SR1_ADDR)) {
        LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
        return false;
    }

    prv_ClearAddrFlag();
    LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
    return true;
}

// ============================================================================
// ЗАПИСЬ В РЕГИСТРЫ
// ============================================================================
bool BSP_I2C_WriteReg(uint8_t dev_addr_7bit, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
    if ((data == 0) && (len > 0U)) {
        return false;
    }

    if (!prv_WaitFlagReset(I2C_SR2_BUSY)) {
        return false;
    }

    BSP_I2C_RTC->SR1 &= ~I2C_SR1_AF;

    // START
    LL_I2C_GenerateStartCondition(BSP_I2C_RTC);
    if (!prv_WaitFlagSet(I2C_SR1_SB)) {
        goto fail;
    }

    // Адрес (Write)
    LL_I2C_TransmitData8(BSP_I2C_RTC, (uint8_t)(dev_addr_7bit << 1U));
    if (!prv_WaitFlagSet(I2C_SR1_ADDR)) {
        goto fail;
    }
    prv_ClearAddrFlag();

    // Адрес регистра
    if (!prv_WaitFlagSet(I2C_SR1_TXE)) {
        goto fail;
    }
    LL_I2C_TransmitData8(BSP_I2C_RTC, reg_addr);

    // Данные
    for (uint16_t i = 0U; i < len; i++) {
        if (!prv_WaitFlagSet(I2C_SR1_TXE)) {
            goto fail;
        }
        LL_I2C_TransmitData8(BSP_I2C_RTC, data[i]);
    }

    // Ожидание BTF (последний байт ушёл в сдвиговый регистр)
    if (!prv_WaitFlagSet(I2C_SR1_BTF)) {
        goto fail;
    }

    LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
    return true;

fail:
    LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
    return false;
}

// ============================================================================
// ЧТЕНИЕ ИЗ РЕГИСТРОВ (ПОСЛЕДОВАТЕЛЬНОСТЬ RM0008 ДЛЯ STM32F1)
// ============================================================================
bool BSP_I2C_ReadReg(uint8_t dev_addr_7bit, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    if ((data == 0) || (len == 0U)) {
        return false;
    }

    if (!prv_WaitFlagReset(I2C_SR2_BUSY)) {
        return false;
    }

    BSP_I2C_RTC->SR1 &= ~I2C_SR1_AF;
    LL_I2C_AcknowledgeNextData(BSP_I2C_RTC, LL_I2C_ACK);

    // START + адрес (Write) — фаза записи адреса регистра
    LL_I2C_GenerateStartCondition(BSP_I2C_RTC);
    if (!prv_WaitFlagSet(I2C_SR1_SB)) {
        goto fail;
    }
    LL_I2C_TransmitData8(BSP_I2C_RTC, (uint8_t)(dev_addr_7bit << 1U));
    if (!prv_WaitFlagSet(I2C_SR1_ADDR)) {
        goto fail;
    }
    prv_ClearAddrFlag();

    // Адрес регистра
    if (!prv_WaitFlagSet(I2C_SR1_TXE)) {
        goto fail;
    }
    LL_I2C_TransmitData8(BSP_I2C_RTC, reg_addr);
    if (!prv_WaitFlagSet(I2C_SR1_BTF)) {
        goto fail;
    }

    // REPEATED START + адрес (Read)
    LL_I2C_GenerateStartCondition(BSP_I2C_RTC);
    if (!prv_WaitFlagSet(I2C_SR1_SB)) {
        goto fail;
    }
    LL_I2C_TransmitData8(BSP_I2C_RTC, (uint8_t)((dev_addr_7bit << 1U) | 0x01U));
    if (!prv_WaitFlagSet(I2C_SR1_ADDR)) {
        goto fail;
    }

    // --- Чтение len байтов ---
    if (len == 1U) {
        // 1 байт: NACK + STOP до сброса ADDR
        LL_I2C_AcknowledgeNextData(BSP_I2C_RTC, LL_I2C_NACK);
        prv_ClearAddrFlag();
        LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
        if (!prv_WaitFlagSet(I2C_SR1_RXNE)) {
            return false;
        }
        data[0] = LL_I2C_ReceiveData8(BSP_I2C_RTC);
        return true;
    }

    if (len == 2U) {
        // 2 байта: ACK, ждём BTF, NACK+STOP, читаем два байта
        LL_I2C_AcknowledgeNextData(BSP_I2C_RTC, LL_I2C_ACK);
        prv_ClearAddrFlag();
        if (!prv_WaitFlagSet(I2C_SR1_BTF)) {
            goto fail;
        }
        LL_I2C_AcknowledgeNextData(BSP_I2C_RTC, LL_I2C_NACK);
        LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
        data[0] = LL_I2C_ReceiveData8(BSP_I2C_RTC);
        data[1] = LL_I2C_ReceiveData8(BSP_I2C_RTC);
        return true;
    }

    // len >= 3: читаем len-3 байтов, затем 3 последних по особой схеме
    prv_ClearAddrFlag();

    uint16_t remaining = len;
    uint16_t idx = 0U;

    while (remaining > 3U) {
        if (!prv_WaitFlagSet(I2C_SR1_RXNE)) {
            goto fail;
        }
        data[idx++] = LL_I2C_ReceiveData8(BSP_I2C_RTC);
        remaining--;
    }

    // Осталось ровно 3 байта
    if (!prv_WaitFlagSet(I2C_SR1_BTF)) {  // байты N-2, N-1 в DR/сдвиг. регистре
        goto fail;
    }
    LL_I2C_AcknowledgeNextData(BSP_I2C_RTC, LL_I2C_NACK);
    data[idx++] = LL_I2C_ReceiveData8(BSP_I2C_RTC);  // N-2

    if (!prv_WaitFlagSet(I2C_SR1_BTF)) {
        goto fail;
    }
    LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
    data[idx++] = LL_I2C_ReceiveData8(BSP_I2C_RTC);  // N-1
    data[idx] = LL_I2C_ReceiveData8(BSP_I2C_RTC);    // N
    return true;

fail:
    LL_I2C_GenerateStopCondition(BSP_I2C_RTC);
    return false;
}