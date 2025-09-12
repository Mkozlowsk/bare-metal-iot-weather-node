/******************************************************************************
  * @file          clocks_bitmap.c
  * @brief         Implementacja sterownika do konfiguracji i zarzadzania systemem taktowania MCU STM32L476.
  *
  * @details       Plik zawiera definicje funkcji odpowiedzialnych za dynamiczne
  *                przelaczanie zrodel zegara (MSI, HSE, PLL). Zawiera takze
  *                implementacje mechanizmu sledzenia zaleznosci uzycia peryferiow,
  *                zegarow i busow.
  *
  * @author        Mateusz Kozlowski
  * @date          08.09.2025
  * @version       v2.0
  *****************************************************************************/

#include "clocks_bitmap.h"

// Stan uzycia zasobów
static uint32_t clock_usage[CLOCK_COUNT];
static uint32_t peripheral_usage[PERIPH_COUNT];
static uint32_t bus_usage[BUS_COUNT];



void CLK_BitmapInit(void) {
    for (int i = 0; i < CLOCK_COUNT; i++) clock_usage[i] = 0;
    for (int i = 0; i < PERIPH_COUNT; i++) peripheral_usage[i] = 0;
    for (int i = 0; i < BUS_COUNT; i++) bus_usage[i] = 0;
}

// Funkcje wewnętrzne
static App_StatusTypeDef CLK_Acquire_Clock(ClockId_t clk) {
    if (clk >= CLOCK_COUNT) return APP_INVALID_PARAM;
    // Jesli zegar juz dziala funkcja przerywa swoje dzialanie. zapobiega to podwojnej inkrementacji licznika zaleznosci przez
    // podwojne wywolanie funkcji inicjujacej 
    if (clock_usage[clk]!= 0) return APP_ALREADY_ACQUIRED;

    if (clock_usage[clk] == 0) { 
        switch (clk) {
            case CLOCK_PLL:{
                PLL_Source_t source_pll = RCC_PLLCLK_GetSource();
                switch (source_pll)
                {
                case PLL_SRC_HSE:
                    if(clock_usage[CLOCK_HSE] == 0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                    else clock_usage[CLOCK_HSE]++;
                    break;
                case PLL_SRC_MSI:
                    if(clock_usage[CLOCK_MSI] == 0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                    else clock_usage[CLOCK_MSI]++;
                    break;
                case PLL_SRC_OTHER: return APP_INVALID_PARAM;
                }
                break;}
            case CLOCK_SYS:{
                SYSCLK_Source_t source_sysclk = SYSCLK_GetSource();
                switch (source_sysclk)
                {
                case SYSCLK_SRC_HSE:
                    if(clock_usage[CLOCK_HSE] == 0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                    else clock_usage[CLOCK_HSE]++;
                    break;
                case SYSCLK_SRC_MSI:
                    if(clock_usage[CLOCK_MSI] == 0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                    else clock_usage[CLOCK_MSI]++;
                    break;
                case SYSCLK_SRC_PLL:
                    if(clock_usage[CLOCK_PLL] == 0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                    else clock_usage[CLOCK_PLL]++;
                    break;
                case SYSCLK_SRC_OTHER: return APP_INVALID_PARAM;
                }
            break;}
            default: break;
        }
        clock_usage[clk]++; //zwieksz licznik dependency dla danego zegara
    }

    return APP_OK;
}

static App_StatusTypeDef CLK_Acquire_Periph(PeripheralId_t periph) {
    // sprawdz czy juz acquired do implementacji
    
    if (periph >= PERIPH_COUNT) return APP_INVALID_PARAM;
    

    if (peripheral_usage[periph] == 0); // do implementacji
    
    peripheral_usage[periph] |= (1u << periph);
    return APP_OK;
}

static App_StatusTypeDef CLK_Acquire_Bus(BusId_t bus) {
    // sprawdz czy juz acquired do implementacji
    if (bus >= BUS_COUNT) return APP_INVALID_PARAM;
    
    uint32_t before = bus_usage[bus];
    bus_usage[bus] |= (1u << bus);

    if (before == 0) {
        switch (bus) {
            case BUS_APB1:
                RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
                __NOP();
                __NOP();
                break;
            default: break;
        }
    }
    return APP_OK;
}

static App_StatusTypeDef CLK_Acquire_Raw(volatile uint32_t *reg, uint32_t mask) {
    // sprawdz czy juz acquired do implementacji
    
    *reg |= mask;
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Clock(ClockId_t clk, uint32_t timeout) {
    if (clk >= CLOCK_COUNT) return APP_INVALID_PARAM;
    
    clock_usage[clk] &= ~(1u << clk);
    if (clock_usage[clk] == 0) {
        switch (clk) {
            case CLOCK_MSI:
                RCC->CR &= ~RCC_CR_MSION;
                while((RCC->CR & RCC_CR_MSIRDY)!=0){        //oczekiwanie na wylaczenie zegara
                    if(--timeout == 0) return APP_TIMEOUT;
                }
                break;
            case CLOCK_HSE:
                RCC->CR &= ~RCC_CR_HSEON;
                while((RCC->CR & RCC_CR_HSERDY)!=0){
                    if(--timeout == 0) return APP_TIMEOUT;
                }
                break;
            case CLOCK_LSI:
                RCC->CSR &= ~RCC_CSR_LSION;
                while((RCC->CSR & RCC_CSR_LSIRDY)!=0){
                    if(--timeout == 0) return APP_TIMEOUT;
                }
                break;
            case CLOCK_LSE:
                RCC->BDCR &= ~RCC_BDCR_LSEON;
                while((RCC->BDCR & RCC_BDCR_LSERDY)!=0){
                    if(--timeout == 0) return APP_TIMEOUT;
                }
                break;
            default: break;
        }
    }
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Periph(PeripheralId_t periph, uint32_t timeout) {
    if (periph >= PERIPH_COUNT) return APP_INVALID_PARAM;
    
    peripheral_usage[periph] &= ~(1u << periph);
    if (peripheral_usage[periph] == 0) { // do implementacji
    }
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Bus(BusId_t bus, uint32_t timeout) {
    if (bus >= BUS_COUNT) return APP_INVALID_PARAM;
    
    bus_usage[bus] &= ~(1u << bus);
    if (bus_usage[bus] == 0) {
        switch (bus) {
            case BUS_APB1: RCC->APB1ENR1 &= ~RCC_APB1ENR1_PWREN; break; //do sprawdzenia czaas na wylaczenie
            default: break;
        }
    }
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Raw(volatile uint32_t *reg, uint32_t mask, uint32_t timeout) {
    *reg &= ~mask;
    return APP_OK;
}

// Funkcje publiczne
App_StatusTypeDef CLK_Acquire(AcquireType_t type, AcquireTarget_t target) {
    switch (type) {
        case ACQUIRE_TYPE_CLOCK: return CLK_Acquire_Clock(target.clock);
        case ACQUIRE_TYPE_PERIPH: return CLK_Acquire_Periph(target.periph);
        case ACQUIRE_TYPE_BUS: return CLK_Acquire_Bus(target.bus);
        case ACQUIRE_TYPE_RAW: return CLK_Acquire_Raw(target.raw.reg, target.raw.mask);
        default: return APP_INVALID_PARAM;
    }
}

App_StatusTypeDef CLK_Release(AcquireType_t type, AcquireTarget_t target, uint32_t timeout) {
    switch (type) {
        case ACQUIRE_TYPE_CLOCK: return CLK_Release_Clock(target.clock, timeout);
        case ACQUIRE_TYPE_PERIPH: return CLK_Release_Periph(target.periph, timeout);
        case ACQUIRE_TYPE_BUS: return CLK_Release_Bus(target.bus, timeout);
        case ACQUIRE_TYPE_RAW: return CLK_Release_Raw(target.raw.reg, target.raw.mask, timeout);
        default: return APP_INVALID_PARAM;
    }
}

uint32_t CLK_GetClockBitmap(ClockId_t clk) {
    return (clk < CLOCK_COUNT) ? clock_usage[clk] : 0;
}

uint32_t CLK_GetPeriphBitmap(PeripheralId_t periph) {
    return (periph < PERIPH_COUNT) ? peripheral_usage[periph] : 0;
}

uint32_t CLK_GetBusBitmap(BusId_t bus) {
    return (bus < BUS_COUNT) ? bus_usage[bus] : 0;
}