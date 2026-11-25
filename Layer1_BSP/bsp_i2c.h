#ifndef BSP_I2C_H
#define BSP_I2C_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// I2C1 (PB8 SCL, PB9 SDA, 100 кГц, 7-битная адресация)
// ============================================================================
// BSP_I2C_Init() доступна ТОЛЬКО из bsp_init.c (защита BSP_INIT_INTERNAL).
// ============================================================================

#ifdef BSP_INIT_INTERNAL
void BSP_I2C_Init(void);
#endif

/**
 * @brief Проверка готовности устройства на шине (аналог HAL_I2C_IsDeviceReady).
 * @param dev_addr_7bit 7-битный адрес устройства.
 * @return true, если устройство ответило ACK.
 */
bool BSP_I2C_IsDeviceReady(uint8_t dev_addr_7bit);

/**
 * @brief Запись массива байтов в регистры I2C-устройства.
 * @param dev_addr_7bit 7-битный адрес устройства.
 * @param reg_addr Адрес внутреннего регистра.
 * @param data Указатель на буфер данных.
 * @param len Количество байтов.
 * @return true при успехе.
 */
bool BSP_I2C_WriteReg(uint8_t dev_addr_7bit, uint8_t reg_addr, const uint8_t *data, uint16_t len);

/**
 * @brief Чтение массива байтов из регистров I2C-устройства.
 * @param dev_addr_7bit 7-битный адрес устройства.
 * @param reg_addr Адрес начального регистра.
 * @param data Указатель на буфер приёма.
 * @param len Количество байтов.
 * @return true при успехе.
 */
bool BSP_I2C_ReadReg(uint8_t dev_addr_7bit, uint8_t reg_addr, uint8_t *data, uint16_t len);

#endif  // BSP_I2C_H