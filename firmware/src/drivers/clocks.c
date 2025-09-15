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
  * @note          Funkcja uzywa sledzenia zaleznosci z clocks_bitmap.h
  * @author        Mateusz Kozlowski
  * @date          01.09.2025
  * @version       v1.0
  *****************************************************************************/
#include "clocks_bitmap.h"
#include "clocks.h"

/**
  * @brief         Inicjalizacja oscylatora MSI.
  * @details       Funkcja konfiguruje zakres MSI, włącza oscylator i czeka na gotowość.
  * @note          Range MSI mozna konfigurowac jedynie gdy zegar jest wylaczony, lub kiedy jest wlaczony i w stanie READY
  * @note          Maksymalna wartosc msi_range to 0xB
  */
App_StatusTypeDef RCC_MSI_Init(uint8_t msi_range, uint32_t timeout){
    if(msi_range > 0xB) return APP_INVALID_PARAM;
    uint32_t timeout_enable = timeout, timeout_disable = timeout;

    //Sprawdzenie zaleznosci zegara przed uruchomieniem
    // Jesli acquire sie nie udal funkcja przerywa dzialanie
    App_StatusTypeDef status = CLK_ACQUIRE_CLOCK(CLOCK_MSI);
    if (status != APP_OK) return status; 

    RCC->CR &= ~RCC_CR_MSION;

    // Oczekiwanie na wylaczenie MSI
    while((RCC->CR & RCC_CR_MSIRDY) != 0){
      if(--timeout_disable == 0) 
      {
        status = CLK_RELEASE_CLOCK(CLOCK_MSI);
        if (status != APP_OK) return status;
        else return APP_TIMEOUT;
      }
    }

    // Ustawienie czestotliwosci po standby i w normalnej operacji
    RCC->CSR &= ~RCC_CSR_MSISRANGE;
    RCC->CSR |= (msi_range << RCC_CSR_MSISRANGE_Pos); 
    RCC->CR &= ~RCC_CR_MSIRANGE;
    RCC->CR |= (msi_range << RCC_CR_MSIRANGE_Pos);

    RCC->CR |= RCC_CR_MSION;
    
    // Oczekiwanie na wlaczenie MSI
    while((RCC->CR & RCC_CR_MSIRDY) == 0){
      if(--timeout_enable == 0) 
      {
        status = CLK_RELEASE_CLOCK(CLOCK_MSI);
        if (status != APP_OK) return status;
        else return APP_TIMEOUT;
      }
    }
    
    return APP_OK;
}

/**
  * @brief         Deinicjalizacja oscylatora MSI.
  * @details       Funkcja wylacza oscylator MSI jesli nie ma zadnego zaleznego od niego peryferium.
  */

App_StatusTypeDef RCC_MSI_Deinit(uint32_t timeout){
  App_StatusTypeDef status = CLK_RELEASE_CLOCK(CLOCK_MSI);
    if (status != APP_OK) return status; 

    RCC->CR &= ~RCC_CR_MSION;

    // Oczekiwanie na wylaczenie MSI
    while((RCC->CR & RCC_CR_MSIRDY) != 0){
      if(--timeout == 0) 
      {
        return APP_TIMEOUT;
      }
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
  
  uint32_t timeout_enable = timeout, timeout_disable = timeout;
  //Sprawdzenie zaleznosci zegara przed uruchomieniem
  App_StatusTypeDef status = CLK_ACQUIRE_CLOCK(CLOCK_HSE);
  if (status != APP_OK) return status;
  
  RCC->CR &= ~RCC_CR_HSEON;

  // Oczekiwanie na wylaczenie HSE
  while((RCC->CR & RCC_CR_HSERDY) != 0){ 
      if(--timeout_disable == 0){
      status = CLK_RELEASE_CLOCK(CLOCK_HSE);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
  }

  RCC->CR &= ~RCC_CR_HSEBYP;
  RCC->CR |= bypass << RCC_CR_HSEBYP_Pos;

  RCC->CR |= RCC_CR_HSEON;

  // Oczekiwanie na wlaczenie HSE
  while((RCC->CR & RCC_CR_HSERDY) == 0){
    if(--timeout_enable == 0){
      status = CLK_RELEASE_CLOCK(CLOCK_HSE);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
  }

  return APP_OK;
}
/**
  * @brief         Deinicjalizacja zewnetrznego oscylatora HSE.
  */
App_StatusTypeDef RCC_HSE_Deinit(bool bypass, uint32_t timeout){
  
  //Sprawdzenie zaleznosci zegara przed wylaczeniem
  App_StatusTypeDef status = CLK_RELEASE_CLOCK(CLOCK_HSE);
  if (status != APP_OK) return status;
  RCC->CR &= ~RCC_CR_HSEON;

  // Oczekiwanie na wylaczenie HSE
  while((RCC->CR & RCC_CR_HSERDY) != 0){ 
      if(--timeout == 0){
      return APP_TIMEOUT;
    }
  }
  return APP_OK;
}


/**
  * @brief         Inicjalizacja PLL.
  * @details       Funkcja konfiguruje zrodlo PLL i okresla wyjsciowa czestotliwosc za pomoca mnoznikow i dzielnikow
  * @note          Na rzecz potrzeb aplikacji uzywane jest tylko jedno wyjscie PLL
  * @note          Wejscie PLL musi miec czestotliwosc w zakresie od 4 do 16 MHz
  * @note          PLL musi być wyłączony podczas konfiguracji, natomiast bit ENABLE moze byc zmieniany w dowolnym momencie.
  * @note          !wymagana jest zmiana #define HSE_FREQ na podstawie uzywanego hardware!
  */
App_StatusTypeDef RCC_PLLCLK_Init(PLL_Source_t source, uint8_t m, uint8_t n, uint8_t r, uint32_t timeout){
  
  
  // Kontrola niewlasciwych parametrow
  if(source!= PLL_SRC_HSE && source!= PLL_SRC_MSI) return APP_INVALID_PARAM;
  if(m < 1 || m > 8) return APP_INVALID_PARAM;
  if(n < 8 || n > 86) return APP_INVALID_PARAM;
  if(r != 2 && r != 4 && r != 6 && r!=8) return APP_INVALID_PARAM;


  

  uint32_t timeout_disable = timeout, timeout_enable = timeout;


  uint32_t pllclk_input_freq = 0;
  if(source == PLL_SRC_MSI) pllclk_input_freq = RCC_MSI_GetFreq();
  if(source == PLL_SRC_HSE) pllclk_input_freq = HSE_FREQ;
  uint32_t pllclk_output_freq = RCC_PLLCLK_CalculateFrequency(pllclk_input_freq, m, n, r);

  if(pllclk_output_freq > 80000000) return APP_ERROR; // czestotliwosc przekroczyla zakres

  App_StatusTypeDef status = CLK_ACQUIRE_CLOCK(CLOCK_PLL);
  if (status != APP_OK) return status;

  RCC->CR &= ~RCC_CR_PLLON;

  // Oczekiwanie na wylaczenie PLL
  while((RCC->CR & RCC_CR_PLLRDY) != 0){
    if(--timeout_disable == 0) {
      status = CLK_RELEASE_CLOCK(CLOCK_PLL);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
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
    if(--timeout_enable == 0) {
      status = CLK_RELEASE_CLOCK(CLOCK_PLL);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
  }

  return APP_OK;
}

App_StatusTypeDef RCC_PLLCLK_Deinit(uint32_t timeout){
  App_StatusTypeDef status = CLK_RELEASE_CLOCK(CLOCK_PLL);
  if (status != APP_OK) return status;

  RCC->CR &= ~RCC_CR_PLLON;

  // Oczekiwanie na wylaczenie PLL
  while((RCC->CR & RCC_CR_PLLRDY) != 0){ 
    if(--timeout == 0) {
      return APP_TIMEOUT;
    }
  }
  return APP_OK;
}


/**
  * @brief         Wybór źródła clocka systemowego.
  * @param source  Źródło clocka do wyboru
  * @note          Funkcja czeka na gotowość źródła przed przełączeniem i na
  *                potwierdzenie przełączenia przez hardware.
  */
App_StatusTypeDef RCC_SYSCLK_SelectSource(SYSCLK_Source_t source, uint32_t timeout)
{
  // Sprawdzenie czy zrodlo jest gotowe
  switch(source) {
      case SYSCLK_SRC_MSI:
          if(!(RCC->CR & RCC_CR_MSIRDY)) return APP_NOT_READY;
          break;
      case SYSCLK_SRC_HSE:
          if(!(RCC->CR & RCC_CR_HSERDY)) return APP_NOT_READY;
          break;
      case SYSCLK_SRC_PLL:
          if(!(RCC->CR & RCC_CR_PLLRDY)) return APP_NOT_READY;
          break;
      case SYSCLK_SRC_OTHER: return APP_INVALID_PARAM;
      default:
          return APP_INVALID_PARAM;
  }
  
 

  RCC->CFGR &= ~RCC_CFGR_SW;
  switch(source) {
      case SYSCLK_SRC_HSE:
          RCC->CFGR |= RCC_CFGR_SW_HSE;
          break;
      case SYSCLK_SRC_MSI:
          RCC->CFGR |= RCC_CFGR_SW_MSI;
          break;
      case SYSCLK_SRC_PLL:  
          RCC->CFGR |= RCC_CFGR_SW_PLL;
          break;
      case SYSCLK_SRC_OTHER: return APP_INVALID_PARAM;
  }

   // Sprawdzenie czy zaleznosci
  App_StatusTypeDef status = CLK_ACQUIRE_CLOCK(CLOCK_SYS);
  if (status != APP_OK) return status;
  
  uint32_t sws_mask;
  switch(source) {
      case SYSCLK_SRC_HSE: sws_mask = RCC_CFGR_SWS_HSE; break;
      case SYSCLK_SRC_MSI: sws_mask = RCC_CFGR_SWS_MSI; break;
      case SYSCLK_SRC_PLL: sws_mask = RCC_CFGR_SWS_PLL; break;
      case SYSCLK_SRC_OTHER: return APP_INVALID_PARAM;
      default: return APP_INVALID_PARAM;
  }
  
  while((RCC->CFGR & RCC_CFGR_SWS) != sws_mask) {
      if(--timeout == 0) {
        return APP_TIMEOUT;
      }
  }

  return APP_OK;
}


/**
  * @brief         Inicjalizacja LSI.
  */
App_StatusTypeDef RCC_LSI_Init(uint32_t timeout){
  App_StatusTypeDef status = CLK_ACQUIRE_CLOCK(CLOCK_LSI);
  if (status != APP_OK) return status;
  
  RCC->CR |= RCC_CSR_LSION;
  while((RCC->CSR & RCC_CSR_LSIRDY) == 0){
    if(--timeout == 0) {
      status = CLK_RELEASE_CLOCK(CLOCK_LSI);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
  }

  return APP_OK;
}

/**
  * @brief         Deinicjalizacja LSI.
  */
App_StatusTypeDef RCC_LSI_Deinit(uint32_t timeout){
  App_StatusTypeDef status = CLK_RELEASE_CLOCK(CLOCK_LSI);
  if (status != APP_OK) return status;
  
  RCC->CR &= ~RCC_CSR_LSION;
  while((RCC->CSR & RCC_CSR_LSIRDY) != 0){
    if(--timeout == 0) {
      return APP_TIMEOUT;
    }
  }
  return APP_OK;
}

/**
  * @brief         Inicjalizacja LSE.
  * @note          Po wlaczeniu LSE mozna jedynie zmniejszyc drive
  */
App_StatusTypeDef RCC_LSE_Init(bool bypass, LSE_XTAL_Drive_t drive, uint32_t timeout){
  uint32_t timeout_disable = timeout, timeout_enable = timeout;

  App_StatusTypeDef status = CLK_ACQUIRE_CLOCK(CLOCK_LSE);
  if (status != APP_OK) return status;  
  
  RCC->BDCR &= ~RCC_BDCR_LSEON;
  while((RCC->BDCR & RCC_BDCR_LSEON)==0){
    if(--timeout_disable == 0) {
      status = CLK_RELEASE_CLOCK(CLOCK_LSE);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
  }
  
  uint8_t drive_val = 0x0;
  switch (drive)
  {
  case LSE_DRIVE_LOW: drive_val = 0x0;break;
  case LSE_DRIVE_MEDIUM_LOW: drive_val = 0x1;break;
  case LSE_DRIVE_MEDIUM_HIGH: drive_val = 0x2;break;
  case LSE_DRIVE_HIGH: drive_val = 0x3;break;
  }

  RCC->BDCR &= ~RCC_BDCR_LSEDRV;
  RCC->BDCR |= (drive_val<<RCC_BDCR_LSEDRV_Pos); 
  
  RCC->BDCR |= RCC_BDCR_LSEON;
  while((RCC->BDCR & RCC_BDCR_LSEON)!=0){
    if(--timeout_enable == 0) {
      status = CLK_RELEASE_CLOCK(CLOCK_LSE);
      if (status != APP_OK) return status;
      return APP_TIMEOUT;
    }
  }

  return APP_OK;
}

/**
  * @brief         Deinicjalizacja LSE.
  */
App_StatusTypeDef RCC_LSE_Deinit(uint32_t timeout){
  App_StatusTypeDef status = CLK_RELEASE_CLOCK(CLOCK_LSE);
  if (status != APP_OK) return status;  
  
  RCC->BDCR &= ~RCC_BDCR_LSEON;
  while((RCC->BDCR & RCC_BDCR_LSEON)!=0){
    if(--timeout == 0) {
      return APP_TIMEOUT;
    }
  }
  return APP_OK;
}

/**
  * @brief         Zmiana drive LSE.
  */
App_StatusTypeDef RCC_LSE_ChangeDrive(LSE_XTAL_Drive_t drive, uint32_t timeout){
  LSE_XTAL_Drive_t drive_current = RCC_LSE_GetDrive();

  if(drive == drive_current) return APP_OK;
  RCC->BDCR &= ~RCC_BDCR_LSEDRV;
  if(drive < drive_current) {
    RCC->BDCR |= (drive << RCC_BDCR_LSEDRV_Pos);
  }
  if(drive > drive_current) {
    uint32_t timeout_disable = timeout;
    uint32_t timeout_enable = timeout;

    RCC->BDCR &= ~RCC_BDCR_LSEON;
    while((RCC->BDCR & RCC_BDCR_LSEON)==0){
      if(--timeout_disable == 0) return APP_TIMEOUT;
    }
    
    RCC->BDCR |= (drive << RCC_BDCR_LSEDRV_Pos);

    RCC->BDCR |= RCC_BDCR_LSEON;
    while((RCC->BDCR & RCC_BDCR_LSEON)!=0){
      if(--timeout_enable == 0) return APP_TIMEOUT;
    } 
  }
  return APP_OK;
}
/**
 *  @brief    Inicjalizacja RTC
 *  @note     System musi byc zawsze skonfigurowany aby osiagnac czestotliwosc PCLK wieksza lub rowna RTCCLK
 *  @note     Po ustawieniu zrodla mozna je zmienic tylko po zresetowaniu backup domain bitem BDRST
 *  @note     Po resecie bity BDRC sa chronione przed zmianami po resecie, aby je modyfikowac nalezy:
 *            1. Wlaczyc APB1 przez ustawienie bitow PWREN w RCC_APB1ENR1
 *            2. ustwawic bit DBP w PWR_CR1
 *            3. Ustawic zrodlo RTC i wlaczyc RTC
 *  @note     4. Po wlaczeniu APB1 nalezy odczekac 2 cykle zegara
 */

App_StatusTypeDef RCC_RTC_Init(RTC_Source_t source){
  bool is_apb1_enabled = 0;
  if(RCC->APB1ENR1 & RCC_APB1ENR1_PWREN) is_apb1_enabled = 1;
  else {
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    __NOP();
    __NOP();
  }

  App_StatusTypeDef status = CLK_ACQUIRE_PERIPH(PERIPH_RTC);
  if (status != APP_OK) return status;  

  PWR->CR1 |= PWR_CR1_DBP;

  RCC->BDCR &= ~RCC_BDCR_RTCEN;

  uint16_t source_val = 0;  if(RCC->APB1ENR1 & RCC_APB1ENR1_PWREN) is_apb1_enabled = 1;

  switch (source) {
  case RTC_SOURCE_LSE:
    if(!(RCC->BDCR & RCC_BDCR_LSERDY)){
      status = CLK_RELEASE_PERIPH(PERIPH_RTC);
      if (status != APP_OK) return status; 
      return APP_NOT_READY;
    }
    source_val = 0x1 << RCC_BDCR_RTCSEL_Pos;
    break;
  case RTC_SOURCE_LSI:
    if(!(RCC->CSR & RCC_CSR_LSIRDY)){
      status = CLK_RELEASE_PERIPH(PERIPH_RTC);
      if (status != APP_OK) return status; 
      return APP_NOT_READY;
    };
    source_val = 0x2 << RCC_BDCR_RTCSEL_Pos;
    break;
  case RTC_SOURCE_HSE:
    if(!(RCC->CR & RCC_CR_HSERDY)){
      status = CLK_RELEASE_PERIPH(PERIPH_RTC);
      if (status != APP_OK) return status; 
      return APP_NOT_READY;
    };
    source_val = 0x3 << RCC_BDCR_RTCSEL_Pos;
    break;
  default:
    PWR->CR1 &= ~PWR_CR1_DBP;
    if(is_apb1_enabled == 0){
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_PWREN; //jesli apb1 bylo wylaczone przed wywolaniem funkcji z powrtoem je wylacz
    __NOP(); 
    __NOP(); 
    }
    return APP_INVALID_PARAM;
  }
  
  RCC->BDCR &= ~RCC_BDCR_RTCSEL;
  RCC->BDCR |= source_val;
  RCC->BDCR |= RCC_BDCR_RTCEN;
  PWR->CR1 &= ~PWR_CR1_DBP;
  if(is_apb1_enabled == 0){
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_PWREN; //jesli apb1 bylo wylaczone przed wywolaniem funkcji z powrtoem je wylacz
    __NOP(); 
    __NOP(); 
  }
  return APP_OK;
}
/* Funkcje diagnostyczne */

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
uint32_t RCC_PLLCLK_GetFrequency(void)
{
    uint32_t pllsrc = RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC;
    
    uint32_t input_freq;
    if(pllsrc == RCC_PLLCFGR_PLLSRC_MSI) {input_freq = RCC_MSI_GetFreq();
    }else if(pllsrc == RCC_PLLCFGR_PLLSRC_HSE) input_freq = HSE_FREQ; else return 0;
    
    uint32_t m = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM)>>RCC_PLLCFGR_PLLM_Pos;
    uint32_t n = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN)>>RCC_PLLCFGR_PLLN_Pos;
    uint32_t r = (RCC->PLLCFGR & RCC_PLLCFGR_PLLR)>>RCC_PLLCFGR_PLLR_Pos;
    
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
  * @brief         Pobiera źródło clocka PLL.
  * @retval        PLL_Source_t Źródło clocka PLL
  */
PLL_Source_t RCC_PLLCLK_GetSource(void){
  uint32_t pllsrc = RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC; 
  
  switch (pllsrc) {
    case RCC_PLLCFGR_PLLSRC_MSI: return PLL_SRC_MSI;
    case RCC_PLLCFGR_PLLSRC_HSE: return PLL_SRC_HSE;
    default: return PLL_SRC_OTHER;
  }
}

/**
  * @brief         Pobiera zrodlo zegara systemowego.
  */
SYSCLK_Source_t SYSCLK_GetSource(void){
  uint8_t sysclk_source = RCC->CFGR & RCC_CFGR_SWS;

  switch (sysclk_source)
  {
  case RCC_CFGR_SWS_HSE: return SYSCLK_SRC_HSE;
  case RCC_CFGR_SWS_MSI: return SYSCLK_SRC_MSI;
  case RCC_CFGR_SWS_PLL: return SYSCLK_SRC_PLL;
  default:  return SYSCLK_SRC_OTHER;
  }
}

/**
  * @brief         Pobiera aktualną częstotliwość clocka systemowego.
  */
uint32_t SYSCLK_GetFreq(void){
  
  SYSCLK_Source_t clock_src = SYSCLK_GetSource();

  switch (clock_src)
  {
  case SYSCLK_SRC_HSE:   return HSE_FREQ;
  case SYSCLK_SRC_MSI: return RCC_MSI_GetFreq();
  case SYSCLK_SRC_PLL: return RCC_PLLCLK_GetFrequency();
  case SYSCLK_SRC_OTHER: return 0;
  default:  return 0;
  }
}

/**
  * @brief         Pobiera aktualną wartosc drive LSE
  */
LSE_XTAL_Drive_t RCC_LSE_GetDrive(void){
  uint8_t drive_val = (RCC->BDCR & RCC_BDCR_LSEDRV) >> RCC_BDCR_LSEDRV_Pos ;
  switch (drive_val)
  {
  case 0x0: return LSE_DRIVE_LOW;
  case 0x1: return LSE_DRIVE_MEDIUM_LOW;
  case 0x2: return LSE_DRIVE_MEDIUM_HIGH;
  case 0x3: return LSE_DRIVE_HIGH;
  default: return LSE_DRIVE_LOW; //Zeby kompilator nie zwracal warninga
  }
  return LSE_DRIVE_LOW; //Zeby kompilator nie zwracal warninga
}

/**
  * @brief         Pobiera aktualne zrodlo RTC; 
 */
RTC_Source_t RCC_RTC_GetSource(void){
  uint16_t source = RCC->BDCR & RCC_BDCR_RTCSEL;
  switch (source) {
  case (0x1 << RCC_BDCR_RTCSEL_Pos): return RTC_SOURCE_LSE;
  case (0x2 << RCC_BDCR_RTCSEL_Pos): return RTC_SOURCE_LSI;
  case (0x3 << RCC_BDCR_RTCSEL_Pos): return RTC_SOURCE_HSE;
  default: return RTC_SOURCE_OTHER;
  }
}
