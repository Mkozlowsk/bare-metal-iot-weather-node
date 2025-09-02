/******************************************************************************
  * @file          clocks.h
  * @brief         Driver do konfiguracji i zarządzania systemem taktowania MCU.
  *
  * @details       Moduł odpowiada za dynamiczne przełączanie między źródłami
  *                clocka (MSI, HSE, PLL) w celu optymalizacji energetycznej.
  *                Zapewnia poprawne sekwencje start/stop dla oscylatorów
  *                oraz konfigurację preskalerów dla magistral AHB, APB1, APB2.
  *
  * @note          Kluczowe dla strategii low-power: MSI dla czujników, HSE+PLL dla radia.
  * @author        Mateusz Kozlowski
  * @date          01.09.2025
  * @version       v1.0
  *****************************************************************************/

#ifndef CLOCKS_H
#define CLOCKS_H

#include "stm32l4xx.h"
#include "stdbool.h"
#include "app_status.h"

/**
  * @brief  Źródła zegaru systemowego.
  */
typedef enum {
    CLOCK_SRC_MSI = 0, /**< Wewnętrzny oscylator multi-speed */
    CLOCK_SRC_HSE,     /**< Zewnętrzny oscylator wysokiej prędkości */
    CLOCK_SRC_PLL,     /**< Pętla fazowa (Phase-Locked Loop) */
    CLOCK_SRC_LSI,     /**< Wewnętrzny oscylator niskiej prędkości */
} ClockSource_t;

/* Funkcje sprawdzające status */
bool HSE_IsReady(void);
bool PLL_IsReady(void);
bool MSI_IsReady(void);
bool LSI_IsReady(void);

/* Funkcje inicjalizacji */
/**
  * @brief         Inicjalizacja oscylatora MSI.
  * @param msi_range Zakres częstotliwości MSI (patrz dokumentacja RCC_CSR_MSISRANGE_*)
  * @param timeout    Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout);

/**
  * @brief         Inicjalizacja oscylatora HSE.
  * @param hse_freq  Częstotliwość HSE w MHz
  * @param bypass    Tryb bypass (true/false)
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_HSE_Init(uint8_t hse_freq, bool bypass);

/**
  * @brief         Inicjalizacja PLL.
  * @param source   Źródło clocka dla PLL
  * @param m        Dzielnik M
  * @param n        Mnożnik N
  * @param p        Dzielnik P
  * @param q        Dzielnik Q
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_PLL_Init(uint8_t source, uint8_t m, uint8_t n, uint8_t p, uint8_t q);

/**
  * @brief         Wybór źródła clocka systemowego.
  * @param source  Źródło clocka do wyboru
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_SYSCLK_SelectSource(ClockSource_t source);

/* Funkcje diagnostyczne */
/**
  * @brief         Pobiera aktualną częstotliwość clocka systemowego.
  * @retval        Częstotliwość w Hz
  */
uint32_t SystemClock_GetSYSCLKFreq(void);

/**
  * @brief         Wyświetla aktualną konfigurację clocków (przez UART).
  */
void SystemClock_PrintConfig(void);

#endif // CLOCKS_H