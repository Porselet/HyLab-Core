#ifndef BSP_FLASH_H
#define BSP_FLASH_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// АППАРАТНЫЙ ДРАЙВЕР ВНУТРЕННЕЙ FLASH STM32F103 (CMSIS, без HAL)
// ============================================================================
// Минимальная единица записи — 16 бит (half-word).
// Стирание — только целой страницей (2 КБ для High-Density).
// ============================================================================

// Адреса двух страниц эмуляции EEPROM (последние 4 КБ Flash)
#define EEPROM_PAGE_0_ADDR (0x0807F000UL)
#define EEPROM_PAGE_1_ADDR (0x0807F800UL)
#define EEPROM_PAGE_SIZE (2048UL)

/**
 * @brief Разблокировка контроллера Flash для записи/стирания.
 */
void BSP_Flash_Unlock(void);

/**
 * @brief Блокировка контроллера Flash (после операций).
 */
void BSP_Flash_Lock(void);

/**
 * @brief Стирание одной страницы Flash (2 КБ).
 * @param page_addr Адрес начала страницы (должен быть кратен 2 КБ).
 * @return true при успехе, false при ошибке.
 * @note Перед вызовом ОБЯЗАТЕЛЬНО вызвать BSP_Watchdog_Feed(),
 *       т.к. стирание занимает ~20-40 мс и ядро приостанавливается.
 */
bool BSP_Flash_ErasePage(uint32_t page_addr);

/**
 * @brief Запись 16-битного half-word по адресу (адрес должен быть чётным).
 * @return true при успехе и совпадении прочитанного значения.
 */
bool BSP_Flash_WriteHalfWord(uint32_t addr, uint16_t data);

/**
 * @brief Запись 32-битного слова как двух half-word (младшее, затем старшее).
 * @return true при успехе.
 */
bool BSP_Flash_WriteWord32(uint32_t addr, uint32_t data);

#endif  // BSP_FLASH_H