#include "bsp_flash.h"

#include "bsp_watchdog.h"
#include "stm32f1xx.h"

// ============================================================================
// РАЗБЛОКИРОВКА / БЛОКИРОВКА FLASH
// ============================================================================
void BSP_Flash_Unlock(void) {
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = 0x45670123UL;
        FLASH->KEYR = 0xCDEF89ABUL;
    }
}

void BSP_Flash_Lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

// ============================================================================
// ОЖИДАНИЕ ЗАВЕРШЕНИЯ ОПЕРАЦИИ И ПРОВЕРКА ОШИБОК
// ============================================================================
static bool prv_WaitReadyAndCheckErrors(void) {
    // Ждём сброса BSY
    while ((FLASH->SR & FLASH_SR_BSY) != 0U) {
        BSP_Watchdog_Feed();
    }

    // Проверка флагов ошибок
    if ((FLASH->SR & (FLASH_SR_PGERR | FLASH_SR_WRPRTERR)) != 0U) {
        // Сбрасываем флаги ошибок записью 1
        FLASH->SR = FLASH_SR_PGERR | FLASH_SR_WRPRTERR;
        return false;
    }
    return true;
}

// ============================================================================
// СТИРАНИЕ СТРАНИЦЫ (2 КБ)
// ============================================================================
bool BSP_Flash_ErasePage(uint32_t page_addr) {
    // Проверка выравнивания на границу страницы
    if ((page_addr & (EEPROM_PAGE_SIZE - 1U)) != 0U) {
        return false;
    }

    // Ждём готовности и сбрасываем старые ошибки
    if (!prv_WaitReadyAndCheckErrors()) {
        return false;
    }

    // Настройка: стирание страницы
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = page_addr;
    FLASH->CR |= FLASH_CR_STRT;

    // Ожидание завершения (с кормлением watchdog)
    bool ok = prv_WaitReadyAndCheckErrors();

    // Сброс бита PER
    FLASH->CR &= ~FLASH_CR_PER;

    return ok;
}

// ============================================================================
// ЗАПИСЬ 16-БИТНОГО HALF-WORD
// ============================================================================
bool BSP_Flash_WriteHalfWord(uint32_t addr, uint16_t data) {
    // Адрес должен быть чётным
    if ((addr & 1U) != 0U) {
        return false;
    }

    // Ждём готовности и сбрасываем старые ошибки
    if (!prv_WaitReadyAndCheckErrors()) {
        return false;
    }

    // Настройка: программирование
    FLASH->CR |= FLASH_CR_PG;

    // Запись half-word (только 16 бит!)
    *(volatile uint16_t *)addr = data;

    // Ожидание завершения (с кормлением watchdog)
    bool ok = prv_WaitReadyAndCheckErrors();

    // Сброс бита PG
    FLASH->CR &= ~FLASH_CR_PG;

    // Проверка прочитанного значения
    if (ok && (*(volatile uint16_t *)addr != data)) {
        ok = false;
    }
    return ok;
}

// ============================================================================
// ЗАПИСЬ 32-БИТНОГО СЛОВА (два half-word)
// ============================================================================
bool BSP_Flash_WriteWord32(uint32_t addr, uint32_t data) {
    if ((addr & 3U) != 0U) {
        return false;
    }
    if (!BSP_Flash_WriteHalfWord(addr, (uint16_t)(data & 0xFFFFU))) {
        return false;
    }
    return BSP_Flash_WriteHalfWord(addr + 2U, (uint16_t)((data >> 16) & 0xFFFFU));
}