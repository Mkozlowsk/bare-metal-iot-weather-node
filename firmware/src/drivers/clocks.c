/******************************************************************************
  * @file          clocks.c
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

#include "clocks.h"
/**
  * @brief         Inicjalizacja oscylatora MSI.
  * @details       Funkcja konfiguruje zakres MSI, włącza oscylator i czeka na gotowość.
  *                Wymagane jest wcześniejsze wyłączenie MSI jeśli był w użyciu.
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout){
    if(msi_range > 0xF) return APP_INVALID_PARAM;
    
    RCC->CR &= ~RCC_CR_MSION;

   
    RCC->CSR &= ~RCC_CSR_MSISRANGE;
    RCC->CSR |= (msi_range << RCC_CSR_MSISRANGE_Pos);
    RCC->CR |= RCC_CR_MSION;

    while((RCC->CR & RCC_CR_MSIRDY) != 1){
        if(--timeout == 0) return APP_TIMEOUT;
    }

    return APP_OK;
}