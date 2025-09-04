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
  * @note          Maksymalna wartosc msi_range to 0xB
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout){
    if(msi_range > 0xB) return APP_INVALID_PARAM;
    
    if((RCC->CR & RCC_CR_MSION) && !(RCC->CR & RCC_CR_MSIRDY)) return APP_NOT_READY; // Jesli MSI jest wlaczone, ale nie jest ready to nie nalezy go modyfikowac
   
    RCC->CSR &= ~RCC_CSR_MSISRANGE;
    RCC->CSR |= (msi_range << RCC_CSR_MSISRANGE_Pos); // Czestotliwosc po standby

    RCC->CR &= ~RCC_CR_MSIRANGE;
    RCC->CR |= (msi_range << RCC_CR_MSIRANGE_Pos);

    if(!(RCC->CR & RCC_CR_MSION)) RCC->CR |= RCC_CR_MSION; //Jesli MSI bylo wylaczone nalezy je wlaczyc
    
    while((RCC->CR & RCC_CR_MSIRDY) == 0){
      if(--timeout == 0) return APP_TIMEOUT;
    }

    return APP_OK;
}

/**
  * @brief         Zwrot wartosci MSI w Hz do wartosci uint16_t
  */
uint32_t RCC_MSI_GetFreq(){
  uint8_t freq = (RCC->CR & RCC_CR_MSIRANGE) >> RCC_CR_MSIRANGE_Pos;
  switch (freq)
  {
  case 0x0:
    return 100000;
    break;
  case 0x1:
    return 200000;
  case 0x2:
    return 400000;
  case 0x3:
    return 800000;
  case 0x4:
    return 1000000;
  case 0x5:
    return 2000000;
  case 0x6:
    return 4000000;
  case 0x7:
    return 8000000;
  case 0x8:
    return 16000000;
  case 0x9:
    return 24000000;
  case 0xA:
    return 32000000;
  case 0xB:
    return 48000000;
  default:
    return 0;
  }
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
  * @brief         Sprawdzenie czy czestotliwosc wyjsciowa PLL nie przekracza zakresu
  * @note          czestotliwosc wyjsciowa w Hz f(VCO) PLL jest ustawiana wedlug:
  *                f(VCO clock) = f(PLL clock input) × (PLLN / PLLM)
  *                f(PLL_R) = f(VCO clock) / PLLR
  */
uint32_t RCC_PLLCLK_CalculateFrequency(uint32_t freq, uint8_t m, uint8_t n, uint8_t r){
  uint32_t frequency = (uint32_t)freq * (uint32_t)n / (uint32_t)m;
  frequency /= r;
  return frequency;
}

/**
  * @brief         Oblicza aktualną częstotliwość PLL na podstawie rejestrów.
  */
static uint32_t RCC_PLLCLK_GetFrequency(void)
{
    uint32_t pllsrc = RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC;
    
    uint32_t input_freq;
    if(pllsrc == RCC_PLLCFGR_PLLSRC_MSI) input_freq = RCC_MSI_GetFreq();
    if(pllsrc == RCC_PLLCFGR_PLLSRC_HSE) input_freq = HSE_FREQ; else return 0;
    
    uint32_t m = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
    uint32_t n = RCC->PLLCFGR & RCC_PLLCFGR_PLLN;
    uint32_t r = RCC->PLLCFGR & RCC_PLLCFGR_PLLR;
    
    uint32_t r_value;
    switch(r) {
        case 0: r_value = 2; break;
        case 1: r_value = 4; break;
        case 2: r_value = 6; break;
        case 3: r_value = 8; break;
        default: r_value = 2;
    }
    
    return (input_freq * n / m /r_value);
}

/**
  * @brief         Inicjalizacja PLL.
  * @details       Funkcja konfiguruje zrodlo PLL i okresla wyjsciowa czestotliwosc za pomoca mnoznikow i dzielnikow
  * @note          Na rzecz potrzeb aplikacji uzywane jest tylko jedno wyjscie PLL
  * @note          Wejscie PLL musi miec czestotliwosc w zakresie od 4 do 16 MHz
  * @note          PLL musi być wyłączony podczas konfiguracji, natomiast bit ENABLE moze byc zmieniany w dowolnym momencie.
  * @note          !wymagana jest zmiana #define HSE_FREQ na podstawie uzywanego hardware!
  */
App_StatusTypeDef RCC_PLLCLK_Init(PLLSource_t source, uint8_t m, uint8_t n, uint8_t r, uint32_t timeout){
  
  
  // Kontrola niewlasciwych parametrow
  if(source!= PLL_SRC_HSE && source!= PLL_SRC_MSI) return APP_INVALID_PARAM;
  if(m < 1 || m > 8) return APP_INVALID_PARAM;
  if(n < 8 || n > 86) return APP_INVALID_PARAM;
  if(r != 2 && r != 4 && r != 6 && r!=8) return APP_INVALID_PARAM;

  uint32_t timeout_disable = timeout;
  uint32_t timeout_enable = timeout;


  uint32_t pllclk_input_freq = 0;
  if(source == PLL_SRC_MSI) pllclk_input_freq = RCC_MSI_GetFreq();
  if(source == PLL_SRC_HSE) pllclk_input_freq = HSE_FREQ;
  uint32_t pllclk_output_freq = RCC_PLLCLK_CalculateFrequency(pllclk_input_freq, m, n, r);

  if(pllclk_output_freq > 80000000) return APP_ERROR; // czestotliwosc przekroczyla zakres
  RCC->CR &= ~RCC_CR_PLLON;

  while((RCC->CR & RCC_CR_PLLRDY) != 0){ // Oczekiwanie na wyzerowanie bitu PLL_Ready
    if(--timeout_disable == 0) return APP_TIMEOUT;
  }

  RCC->PLLCFGR &= ~ RCC_PLLCFGR_PLLM;
  RCC->PLLCFGR |= (m) << RCC_PLLCFGR_PLLM_Pos;

  RCC->PLLCFGR &= ~ RCC_PLLCFGR_PLLN;
  RCC->PLLCFGR |= n << RCC_PLLCFGR_PLLN_Pos;
  
  RCC->PLLCFGR &= ~ RCC_PLLCFGR_PLLR;
  RCC->PLLCFGR |= ((r/2)-1) << RCC_PLLCFGR_PLLR_Pos; //00 -> r=2, 01 -> r=4, 10 -> r = 6 etc.; 
  
  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
  
  if(source == PLL_SRC_HSE) RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
  if(source == PLL_SRC_MSI) RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_MSI;

  RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;

  RCC->CR |= RCC_CR_PLLON;

  while((RCC->CR & RCC_CR_PLLRDY) == 0){
    if(--timeout_enable == 0) return APP_TIMEOUT;
  }

  return APP_OK;
}


/**
  * @brief         Wybór źródła clock  uint8_t m = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM_Msk) >> RCC_PLLCFGR_PLLM_Pos;
  uint8_t n = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN_Msk) >> RCC_PLLCFGR_PLLN_Pos;
  uint8_t r = (RCC->PLLCFGR & RCC_PLLCFGR_PLLR_Msk) >> RCC_PLLCFGR_PLLR_Pos;a systemowego.
  * @param source  Źródło clocka do wyboru
  * @note          Funkcja czeka na gotowość źródła przed przełączeniem i na
  *                potwierdzenie przełączenia przez hardware.
  */
App_StatusTypeDef RCC_SYSCLK_SelectSource(ClockSource_t source, uint32_t timeout)
{
    // Sprawdzenie czy zrodlo jest gotowe
    switch(source) {
        case CLOCK_SRC_MSI:
            if(!(RCC->CR & RCC_CR_MSIRDY)) return APP_NOT_READY;
            break;
        case CLOCK_SRC_HSE:
            if(!(RCC->CR & RCC_CR_HSERDY)) return APP_NOT_READY;
            break;
        case CLOCK_SRC_PLL:
            if(!(RCC->CR & RCC_CR_PLLRDY)) return APP_NOT_READY;
            break;
        case CLOCK_SRC_OTHER: return APP_INVALID_PARAM;
        default:
            return APP_INVALID_PARAM;
    }
    
    RCC->CFGR &= ~RCC_CFGR_SW;
    switch(source) {
        case CLOCK_SRC_HSE:
            RCC->CFGR |= RCC_CFGR_SW_HSE;
            break;
        case CLOCK_SRC_MSI:
            RCC->CFGR |= RCC_CFGR_SW_MSI;
            break;
        case CLOCK_SRC_PLL:  
            RCC->CFGR |= RCC_CFGR_SW_PLL;
            break;
        case CLOCK_SRC_OTHER: return APP_INVALID_PARAM;
    }
    
    // Oczekwianie na potwierdzenie przełączenia przez hardware
    uint32_t sws_mask;
    switch(source) {
        case CLOCK_SRC_HSE: sws_mask = RCC_CFGR_SWS_HSE; break;
        case CLOCK_SRC_MSI: sws_mask = RCC_CFGR_SWS_MSI; break;
        case CLOCK_SRC_PLL: sws_mask = RCC_CFGR_SWS_PLL; break;
        case CLOCK_SRC_OTHER: return APP_INVALID_PARAM;
        default: return APP_INVALID_PARAM;
    }
    
    while((RCC->CFGR & RCC_CFGR_SWS) != sws_mask) {
        if(--timeout == 0) {
            return APP_TIMEOUT; // Przełączenie nie powiodło się
        }
    }
    
    return APP_OK;
}

/* Funkcje diagnostyczne */


/**
  * @brief         Pobiera zrodlo zegara systemowego.
  */
ClockSource_t SystemClock_GetSource(void){
  uint8_t sysclk_source = RCC->CFGR & RCC_CFGR_SWS;

  switch (sysclk_source)
  {
  case RCC_CFGR_SWS_HSE: return CLOCK_SRC_HSE;
  case RCC_CFGR_SWS_MSI: return CLOCK_SRC_MSI;
  case RCC_CFGR_SWS_PLL: return CLOCK_SRC_PLL;
  default:  return CLOCK_SRC_OTHER;
  }
}

/**
  * @brief         Pobiera aktualną częstotliwość clocka systemowego.
  */
uint32_t SystemClock_GetSYSCLKFreq(void){

  ClockSource_t clock_src = SystemClock_GetSource();

  switch (clock_src)
  {
  case CLOCK_SRC_HSE:   return HSE_FREQ;
  case CLOCK_SRC_MSI: return RCC_MSI_GetFreq();
  case CLOCK_SRC_PLL: return RCC_PLLCLK_GetFrequency();
  case CLOCK_SRC_OTHER: return 0;
  default:  return 0;
  }
}
