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
#include "clocks_bitmap.h"

#define HSE_FREQ 48000000// Czestotliwosc HSE w Hz ustawiana na podstawie hardware

/**
  * @brief  Źródła zegaru systemowego.
  */
typedef enum __attribute__((packed)){
    SYSCLK_SRC_MSI = 0, /**< Wewnętrzny oscylator multi-speed */
    SYSCLK_SRC_HSE,     /**< Zewnętrzny oscylator wysokiej prędkości */
    SYSCLK_SRC_PLL,     /**< Pętla fazowa (Phase-Locked Loop) */
    SYSCLK_SRC_OTHER,
} SYSCLK_Source_t;


/* Funkcje inicjalizacji i deinicjalizacji */
/**
  * @brief         Inicjalizacja oscylatora MSI.
  * @param msi_range Zakres częstotliwości MSI (patrz dokumentacja RCC_CSR_MSISRANGE_*)
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  * @note          Maksymalna wartosc msi_range to 0xB
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout);

/**
  * @brief         Deinicjalizacja oscylatora MSI.
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_MSI_Deinit(uint32_t timeout);


/**
  * @brief         Inicjalizacja oscylatora HSE.
  * @param bypass    Tryb bypass (true/false), czyli czy HSE ma byc bypassowane przez zewnetrzny zegar (false - HSE oscylator nie jest bypassowany)
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_HSE_Init(bool bypass, uint32_t timeout);

/**
  * @brief         Deinicjalizacja oscylatora HSE.
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_HSE_Deinit(uint32_t timeout);


/**
  * @brief  Źródla PLL.
  */
typedef enum __attribute__((packed)){
    PLL_SRC_MSI = 0,
    PLL_SRC_HSE,
    PLL_SRC_OTHER,
} PLL_Source_t;

/**
  * @brief         Inicjalizacja PLL.
  * @param source   PLLSource_t - Źródło clocka dla PLL
  * @param m        Dzielnik M
  * @param n        Mnożnik N
  * @param r        Dzielnik R
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_PLLCLK_Init(PLL_Source_t source, uint8_t m, uint8_t n, uint8_t r, uint32_t timeout);

/**
  * @brief         Deinicjalizacja PLL.
  * @param timeout    Timeout w cyklach pętli sysclk
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_PLLCLK_Deinit(uint32_t timeout);

/**
  * @brief         Wybór źródła clocka systemowego.
  * @param source  Źródło clocka do wyboru
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_SYSCLK_SelectSource(SYSCLK_Source_t source, uint32_t timeout);

/**
  * @brief         Inicjalizacja LSI.
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_LSI_Init(uint32_t timeout);

/**
  * @brief         Deinicjalizacja LSI.
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_LSI_Deinit(uint32_t timeout);

/**
  * @brief        Typy drive XTAL dla LSE
  */
typedef enum __attribute__((packed)){
  LSE_DRIVE_LOW,
  LSE_DRIVE_MEDIUM_LOW,
  LSE_DRIVE_MEDIUM_HIGH,
  LSE_DRIVE_HIGH,
}LSE_XTAL_Drive_t;


/**
  * @brief         Inicjalizacja LSE.
  * @param bypass    Tryb bypass (true/false), czyli czy LSE ma byc bypassowane przez zewnetrzny zegar (false - LSE oscylator nie jest bypassowany)
  * @param drive   LSE_XTAL_Drive_t drive capability
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_LSE_Init(bool bypass, LSE_XTAL_Drive_t drive, uint32_t timeout);

/**
  * @brief         Deinicjalizacja LSE.
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_LSE_Deinit(uint32_t timeout);

/**
  * @brief         Zmiana drive LSE.
  * @param drive   LSE_XTAL_Drive_t drive capability
  * @param timeout Timeout w cyklach pętli
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_LSE_ChangeDrive(LSE_XTAL_Drive_t drive, uint32_t timeout);

/**
 * @brief Zrodla RTC
 */
typedef enum __attribute__((packed)){
  RTC_SOURCE_LSE,
  RTC_SOURCE_LSI,
  RTC_SOURCE_HSE,
  RTC_SOURCE_OTHER
}RTC_Source_t;

/**
  * @brief         Inicjalizacja RTC.
  * @param source  RTC_Source_t zrodlo RTC
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_RTC_Init(RTC_Source_t source);

/**
  * @brief         Deinicjalizacja RTC.
  * @retval        App_StatusTypeDef Status operacji
  */
App_StatusTypeDef RCC_RTC_Deinit();

/* Funkcje diagnostyczne */

/**
  * @brief          Obliczenie czestotliwosci PLLCLK
  * @retval         uint32_t Czestotliwosc wyjsciowa PLLCLK w KHz
  */
uint32_t RCC_PLLCLK_CalculateFrequency(uint32_t freq, uint8_t m, uint8_t n, uint8_t r);


/**
  * @brief         Oblicza aktualną częstotliwość PLL na podstawie rejestrów.
  * @retval        Częstotliwość PLL [Hz].
  */
uint32_t RCC_PLLCLK_GetFrequency(void);

/**
  * @brief         Zwraca zrodlo PLL.
  * @retval        PLL_Source_t Zrodlo PLL.
  */
PLL_Source_t RCC_PLLCLK_GetSource(void);

/**typedef enum ClockId_t ClockId_t;
typedef enum BusId_t BusId_t;
typedef enum PeripheralId_t PeripheralId_t;
  * @brief          Zwrot wartosci czestotliwosci zegara MSI w Hz do wartosci uint32_t
  * @retval         uint16_t Częstotliwość w Hz.
  */
uint32_t RCC_MSI_GetFreq();

/**
  * @brief         Pobiera zrodlo zegara systemowego.
  * @retval        ClockSource_t Zrodlo zegara systemowego
  */
SYSCLK_Source_t SYSCLK_GetSource(void);

/* Funkcje diagnostyczne */
/**
  * @brief         Pobiera aktualną częstotliwość clocka systemowego.
  * @retval        Częstotliwość w Hz
  */
uint32_t SYSCLK_GetFreq(void);


/**
  * @brief         Wyświetla aktualną konfigurację clocków (przez UART).
  */
void SystemClock_PrintConfig(void);

/**
  * @brief         Zwraca obecne ustawienie drive LSE
  * @retval        LSE_XTAL_Drive_t stan drive LSE
  */
LSE_XTAL_Drive_t RCC_LSE_GetDrive(void);

/**
  * @brief         Pobiera zrodlo zegara RTC.
  * @retval        RTC_Source_t Zrodlo RTC.
  */
RTC_Source_t RCC_RTC_GetSource(void);

#endif // CLOCKS_H