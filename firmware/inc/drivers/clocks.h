/******************************************************************************
  * @file          clocks.h
  * @brief         Driver do konfiguracji i zarządzania systemem taktowania MCU STM32L476.
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

#define HSE_FREQ 48000000// Czestotliwosc HSE w Hz ustawiana na podstawie hardware

/**
  * @brief  Źródła zegaru systemowego.
  */
typedef enum __attribute__((packed)){
    CLOCK_SRC_MSI = 0, /**< Wewnętrzny oscylator multi-speed */
    CLOCK_SRC_HSE,     /**< Zewnętrzny oscylator wysokiej prędkości */
    CLOCK_SRC_PLL,     /**< Pętla fazowa (Phase-Locked Loop) */
    CLOCK_SRC_OTHER,
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
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  * @note          Maksymalna wartosc msi_range to 0xB
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout);

/**
  * @brief         Inicjalizacja oscylatora HSE.
  * @param bypass    Tryb bypass (true/false), czyli czy HSE ma byc bypassowane przez zewnetrzny zegar (false - HSE oscylator nie jest bypassowany)
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_HSE_Init(bool bypass, uint32_t timeout);

/**
  * @brief  Źródla PLL.
  */
typedef enum __attribute__((packed)){
    PLL_SRC_MSI = 0,
    PLL_SRC_HSE,
} PLLSource_t;








/**
  * @brief         Inicjalizacja PLL.
  * @param source   PLLSource_t - Źródło clocka dla PLL
  * @param m        Dzielnik M
  * @param n        Mnożnik N
  * @param r        Dzielnik R
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_PLLCLK_Init(PLLSource_t source, uint8_t m, uint8_t n, uint8_t r, uint32_t timeout);

/**
  * @brief         Wybór źródła clocka systemowego.
  * @param source  Źródło clocka do wyboru
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_SYSCLK_SelectSource(ClockSource_t source, uint32_t timeout);

/* Funkcje diagnostyczne */

/**
  * @brief          Obliczenie czestotliwosci PLLCLK
  * @retval         uint32_t Czestotliwosc wyjsciowa PLLCLK w KHz
  */
uint32_t RCC_PLLCLK_CalculateFrequency(uint32_t freq, uint8_t m, uint8_t n, uint8_t r);


/**
  * @brief         Oblicza aktualną częstotliwość PLL na podstawie rejestrów.
  * @retval        Częstotliwość PLL [Hz]
  */
static uint32_t RCC_PLLCLK_GetFrequency(void);

/**
  * @brief          Zwrot wartosci czestotliwosci zegara MSI w Hz do wartosci uint32_t
  * @retval         uint16_t Częstotliwość w Hz.
  */
uint32_t RCC_MSI_GetFreq();

/**
  * @brief         Pobiera zrodlo zegara systemowego.
  * @retval        ClockSource_t Zrodlo zegara systemowego
  */
ClockSource_t SystemClock_GetSource(void);

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