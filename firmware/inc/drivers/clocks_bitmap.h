/******************************************************************************
  * @file          clocks_bitmap.h
  * @brief         Sterownik do konfiguracji i zarzadzania systemem taktowania MCU STM32L476.
  *
  * @details       Modul odpowiada za dynamiczne przelaczanie miedzy zrodlami
  *                zegara (MSI, HSE, PLL) w celu optymalizacji energetycznej i 
  *                Bezpieczenstwa uzytkowania.
  *                Dodatkowo implementuje mechanizm bitmapy do sledzenia uzycia
  *                zegarow przez rozne peryferia i bezpiecznego ich wylaczania.
  *
  * @author        Mateusz Kozlowski
  * @date          08.09.2025
  * @version       v1.0
  *****************************************************************************/

#ifndef CLOCKS_BITMAP_H
#define CLOCKS_BITMAP_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32l476xx.h"
#include "app_status.h"

// Typy identyfikatorów
// Typy clocków systemowych
typedef enum {
    CLOCK_MSI = 0,
    CLOCK_HSE,
    CLOCK_LSI,
    CLOCK_LSE,
    CLOCK_COUNT
} ClockId_t;

// Typy magistral
typedef enum {
    BUS_APB1,
    BUS_COUNT
} BusId_t;

// Typy peryferiów
typedef enum {
    PERIPH_PWR = 0,
    PERIPH_RTC,
    PERIPH_COUNT
} PeripheralId_t;

// Typy akwizycji
typedef enum {
    ACQUIRE_TYPE_CLOCK = 0,
    ACQUIRE_TYPE_PERIPH,
    ACQUIRE_TYPE_BUS,
    ACQUIRE_TYPE_RAW
} AcquireType_t;

// Unia celów akwizycji
typedef union {
    ClockId_t clock;
    PeripheralId_t periph;
    BusId_t bus;
    struct {
        volatile uint32_t *reg;
        uint32_t mask;
    } raw;
} AcquireTarget_t;


/**
 * @brief           Uniwersalna funkcja akwizycji zasobów clocka
 * @param           type Typ akwizycji (clock, peryferium, magistrala, raw)
 * @param           target Cel akwizycji
 * @retval          App_StatusTypeDef status aplikacji
 */
App_StatusTypeDef CLK_Acquire(AcquireType_t type, AcquireTarget_t target, uint32_t timeout);

/**
 * @brief           Uniwersalna funkcja zwalniania zasobów clocka
 * @param           type Typ zwolnienia (clock, peryferium, magistrala, raw)
 * @param           target Cel zwolnienia
 * @retval          App_StatusTypeDef status aplikacji
 */
App_StatusTypeDef CLK_Release(AcquireType_t type, AcquireTarget_t target, uint32_t timeout);

/**
 * @brief           Funkcja zwracajaca bitmape uzycia clocka
 * @param           clk Zrodlo zegara
 * @retval          bitmapa uzycia
 */
uint32_t CLK_GetClockBitmap(ClockId_t clk);

/**
 * @brief           Funkcja zwracajaca bitmape uzycia peryferium
 * @param           periph Peryferium
 * @retval          bitmapa uzycia
 */
uint32_t CLK_GetPeriphBitmap(PeripheralId_t periph);

/**
 * @brief           Funkcja zwracajaca bitmape uzycia magistrali
 * @param           bus Magistrala
 * @retval          bitmapa uzycia
 */
uint32_t CLK_GetBusBitmap(BusId_t bus);

/**
 * @brief           Funkcja inicjalizujaca bitmape zaleznosci zegarow
 */
void CLK_BitmapInit(void);

// Makra dla wygody użytkowania
#define CLK_ACQUIRE_CLOCK(clock, timeout) \
    CLK_Acquire(ACQUIRE_TYPE_CLOCK, (AcquireTarget_t){.clock = (clock)}, (uint32_t)(timeout))

#define CLK_ACQUIRE_PERIPH(periph, timeout) \
    CLK_Acquire(ACQUIRE_TYPE_PERIPH, (AcquireTarget_t){.periph = (periph)}, (uint32_t)(timeout))

#define CLK_ACQUIRE_BUS(bus, timeout) \
    CLK_Acquire(ACQUIRE_TYPE_BUS, (AcquireTarget_t){.bus = (bus)}, (uint32_t)(timeout))

#define CLK_ACQUIRE_RAW(reg, mask, timeout) \
    CLK_Acquire(ACQUIRE_TYPE_RAW, (AcquireTarget_t){.raw = {(reg), (mask)}}, (uint32_t)(timeout))

#define CLK_RELEASE_CLOCK(clock, timeout) \
    CLK_Release(ACQUIRE_TYPE_CLOCK, (AcquireTarget_t){.clock = (clock)}, (uint32_t)(timeout))

#define CLK_RELEASE_PERIPH(periph, timeout) \
    CLK_Release(ACQUIRE_TYPE_PERIPH, (AcquireTarget_t){.periph = (periph)}, (uint32_t)(timeout))

#define CLK_RELEASE_BUS(bus, timeout) \
    CLK_Release(ACQUIRE_TYPE_BUS, (AcquireTarget_t){.bus = (bus)}, (uint32_t)(timeout))

#define CLK_RELEASE_RAW(reg, mask, timeout) \
    CLK_Release(ACQUIRE_TYPE_RAW, (AcquireTarget_t){.raw = {(reg), (mask)}}, (uint32_t)(timeout))

#endif // CLOCKS_BITMAP_H
