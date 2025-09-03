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
  * @note          Range MSI mozna konfigurowac jedynie gdy zegar jest wylaczony, lub kiedy jest wlaczony i w stanie READY
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout){
    if(msi_range > 0xF) return APP_INVALID_PARAM;
    
    if((RCC->CR & RCC_CR_MSION) && !(RCC->CR & RCC_CR_MSIRDY)) return APP_NOT_READY; // Jesli MSI jest wlaczone, ale nie jest ready to nie nalezy go modyfikowac
   
    RCC->CSR &= ~RCC_CSR_MSISRANGE;
    RCC->CSR |= (msi_range << RCC_CSR_MSISRANGE_Pos);

    if(!(RCC->CR & RCC_CR_MSION)) RCC->CR |= RCC_CR_MSION; //Jesli MSI bylo wylaczone nalezy je wlaczyc
    
    while((RCC->CR & RCC_CR_MSIRDY) == 0){
      if(--timeout == 0) return APP_TIMEOUT;
    }

    return APP_OK;
}
/**
  * @brief         Inicjalizacja zewnetrznego oscylatora HSE.
  * @details       Funkcja konfiguruje tryb bypass HSE, włącza oscylator i czeka na gotowość.
  * @note          HSE musi być wyłączony podczas konfiguracji.
  * @note          Po wyclearowaniu HSEON HSERDY spada do 0 bo 6 cyklach HSE
  * @note          HSE musi miec czestotliwosc od 4MHz do 16MHZ, ze wzgledu na to, ze dziala jako input PLL
  */
App_StatusTypeDef RCC_HSE_Init(bool bypass, uint32_t timeout){
  
  RCC->CR &= ~RCC_CR_HSEON;

  while((RCC->CR & RCC_CR_HSERDY) != 0){ // Oczekiwanie na wyzerowanie bitu HSE_Ready
    if(--timeout == 0) return APP_TIMEOUT;
  }


  RCC->CR &= ~RCC_CR_HSEBYP;
  RCC->CR |= bypass << RCC_CR_HSEBYP_Pos;

  RCC->CR |= RCC_CR_HSEON;

  while((RCC->CR & RCC_CR_HSERDY) == 0){
    if(--timeout == 0) return APP_TIMEOUT;
  }

  return APP_OK;
}

/**
  * @brief         Inicjalizacja PLL.
  * @details       Funkcja konfiguruje zrodlo PLL i okresla wyjsciowa czestotliwosc za pomoca mnoznikow i dzielnikow
  * @note          Na rzecz potrzeb aplikacji uzywane jest tylko jedno wyjscie PLL
  * @note          Wejscie PLL musi miec czestotliwosc w zakresie od 4 do 16 MHz
  * @note          PLL musi być wyłączony podczas konfiguracji, natomiast bit ENABLE moze byc zmieniany w dowolnym momencie.
  * @note          czestotliwosc wyjsciowa f(VCO) PLL jest ustawiana wedlug:
  *                f(VCO clock) = f(PLL clock input) × (PLLN / PLLM)
  *                (PLL_P) = f(VCO clock) / PLLP
  *                PLL_Q) = f(VCO clock) / PLLQ
  *                f(PLL_R) = f(VCO clock) / PLLR
  */
App_StatusTypeDef RCC_PLL_Init(PLLSource_t source, uint8_t m, uint8_t n, uint8_t p, uint8_t q, uint8_t r, uint32_t timeout){
  RCC->CR &= ~RCC_CR_PLLON;

  while((RCC->CR & RCC_CR_PLLRDY) != 0){ // Oczekiwanie na wyzerowanie bitu PLL_Ready
    if(--timeout == 0) return APP_TIMEOUT;
  }

  RCC->CR |= RCC_CR_PLLON;

  while((RCC->CR & RCC_CR_PLLRDY) == 0){
    if(--timeout == 0) return APP_TIMEOUT;
  }

  return APP_OK;
}
