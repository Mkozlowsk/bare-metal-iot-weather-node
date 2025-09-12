/******************************************************************************
  * @file          clocks_bitmap.c
  * @brief         Implementacja sterownika do konfiguracji i zarzadzania systemem taktowania MCU STM32L476.
  *
  * @details       Plik zawiera definicje funkcji odpowiedzialnych za dynamiczne
  *                przelaczanie zrodel zegara (MSI, HSE, PLL). Zawiera takze
  *                implementacje mechanizmu sledzenia zaleznosci uzycia peryferiow,
  *                zegarow i busow.
  * @note          Ponizszy kod nie ma wplywu na hardware, jedynie sledzi zaleznosci zegarow, peryferiow i busow
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

    switch (clk) {
        case CLOCK_PLL:{
            PLL_Source_t source_pll = RCC_PLLCLK_GetSource();
            switch (source_pll)
            {
            case PLL_SRC_HSE:
                if(clock_usage[CLOCK_HSE] == 0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                else clock_usage[CLOCK_HSE]++; //dodanie zaleznosci do HSE
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

    return APP_OK;
}

static App_StatusTypeDef CLK_Acquire_Periph(PeripheralId_t periph) {
    if(periph >= PERIPH_COUNT) return APP_INVALID_PARAM;
    if(peripheral_usage[periph]!= 0) return APP_ALREADY_ACQUIRED;

    switch (periph)
    {
    case PERIPH_RTC:{
        RTC_Source_t source_rtc = RCC_RTC_GetSource();
        switch (source_rtc)
        {
        case RTC_SOURCE_HSE:
            if(clock_usage[CLOCK_HSE]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_HSE]++;
            break;
        case RTC_SOURCE_LSE:
            if(clock_usage[CLOCK_LSE]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_LSE]++;
            break;
        case RTC_SOURCE_LSI:
            if(clock_usage[CLOCK_LSI]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_LSI]++;
            break;
        case RTC_SOURCE_OTHER: return APP_INVALID_PARAM;
        default:
            break;
        }

        break;}
    case PERIPH_PWR:{
        if(bus_usage[BUS_APB1]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            bus_usage[BUS_APB1]++;
        break;}
    default:
        break;
    }

    peripheral_usage[periph]++;
    return APP_OK;
}

static App_StatusTypeDef CLK_Acquire_Bus(BusId_t bus) {
    if(bus >= BUS_COUNT) return APP_INVALID_PARAM;
    if(bus_usage[bus]!= 0) return APP_ALREADY_ACQUIRED;
    
    switch (bus) {
        case BUS_AHB:
            if(clock_usage[CLOCK_SYS]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_SYS]++;
            break;
        case BUS_APB1:
            if(bus_usage[BUS_APB1]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            bus_usage[BUS_APB1]++;
            break;
        case BUS_APB2:
            if(bus_usage[BUS_APB2]==0) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            bus_usage[BUS_APB2]++;
            break;
        default: break;
    }

    bus_usage[bus]++;
    return APP_OK;
}

static App_StatusTypeDef CLK_Acquire_Raw(volatile uint32_t *reg, uint32_t mask) {
    // sprawdz czy juz acquired do implementacji
    
    *reg |= mask;
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Clock(ClockId_t clk) {
    if (clk >= CLOCK_COUNT) return APP_INVALID_PARAM;
    if(clock_usage[clk] == 0) return APP_ALREADY_RELEASED;
    if(clock_usage[clk] > 1)return APP_DEPENDENCIES_NOT_RELEASED;
    switch (clk) {
        case CLOCK_PLL:{
            PLL_Source_t source_pll = RCC_PLLCLK_GetSource();
            switch (source_pll)
            {
            case PLL_SRC_HSE:
                if(clock_usage[CLOCK_HSE] <= 1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                else clock_usage[CLOCK_HSE]--; //dodanie zaleznosci do HSE
                break;
            case PLL_SRC_MSI:
                if(clock_usage[CLOCK_MSI] <= 1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                else clock_usage[CLOCK_MSI]--;
                break;
            case PLL_SRC_OTHER: return APP_INVALID_PARAM;
            }
            break;}
        case CLOCK_SYS:{
            SYSCLK_Source_t source_sysclk = SYSCLK_GetSource();
            switch (source_sysclk)
            {
            case SYSCLK_SRC_HSE:
                if(clock_usage[CLOCK_HSE] <= 1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                else clock_usage[CLOCK_HSE]--;
                break;
            case SYSCLK_SRC_MSI:
                if(clock_usage[CLOCK_MSI] <= 1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                else clock_usage[CLOCK_MSI]--;
                break;
            case SYSCLK_SRC_PLL:
                if(clock_usage[CLOCK_PLL] <= 1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
                else clock_usage[CLOCK_PLL]--;
                break;
            case SYSCLK_SRC_OTHER: return APP_INVALID_PARAM;
            }
        break;}
        default: break;
    }
    clock_usage[clk]--; //zmniejsz licznik dependency dla danego zegara

    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Periph(PeripheralId_t periph) {
    if(peripheral_usage[periph] == 0) return APP_ALREADY_RELEASED;
    if(peripheral_usage[periph] > 1)return APP_DEPENDENCIES_NOT_RELEASED;

    switch (periph)
    {
    case PERIPH_RTC:{
        RTC_Source_t source_rtc = RCC_RTC_GetSource();
        switch (source_rtc)
        {
        case RTC_SOURCE_HSE:
            if(clock_usage[CLOCK_HSE]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_HSE]--;
            break;
        case RTC_SOURCE_LSE:
            if(clock_usage[CLOCK_LSE]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_LSE]--;
            break;
        case RTC_SOURCE_LSI:
            if(clock_usage[CLOCK_LSI]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_LSI]--;
            break;
        case RTC_SOURCE_OTHER: return APP_INVALID_PARAM;
        default:
            break;
        }

        break;}
    case PERIPH_PWR:{
        if(bus_usage[BUS_APB1]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            bus_usage[BUS_APB1]--;
        break;}
    default:
        break;
    }

    peripheral_usage[periph]--;
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Bus(BusId_t bus) {
    if(bus_usage[bus] == 0) return APP_ALREADY_RELEASED;
    if(bus_usage[bus] > 1)return APP_DEPENDENCIES_NOT_RELEASED;

    switch (bus) {
        case BUS_AHB:
            if(clock_usage[CLOCK_SYS]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            clock_usage[CLOCK_SYS]--;
            break;
        case BUS_APB1:
            if(bus_usage[BUS_APB1]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            bus_usage[BUS_APB1]--;
            break;
        case BUS_APB2:
            if(bus_usage[BUS_APB2]<=1) return APP_DEPENDENT_CLOCK_NOT_CONFIGURED;
            bus_usage[BUS_APB2]--;
            break;
        default: break;
    }

    bus_usage[bus]--;
    return APP_OK;
}

static App_StatusTypeDef CLK_Release_Raw(volatile uint32_t *reg, uint32_t mask) {
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

App_StatusTypeDef CLK_Release(AcquireType_t type, AcquireTarget_t target) {
    switch (type) {
        case ACQUIRE_TYPE_CLOCK: return CLK_Release_Clock(target.clock);
        case ACQUIRE_TYPE_PERIPH: return CLK_Release_Periph(target.periph);
        case ACQUIRE_TYPE_BUS: return CLK_Release_Bus(target.bus);
        case ACQUIRE_TYPE_RAW: return CLK_Release_Raw(target.raw.reg, target.raw.mask);
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