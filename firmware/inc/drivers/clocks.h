#ifndef CLOCKS_H
/*
 * File: clocks.h
 * Description: Clock configuration and utility functions for STM32L4 series.
 *
 * Author: Mateusz Kozlowski
 * Date: 30-08-2025
 * License: MIT
 *
 * Revision History:
 *   - <30-08-2025>: Created initial version.
 *
 */
#define CLOCKS_H

#include "stm32l4xx.h" // Hardware definitions

typedef enum {
    CLOCK_SRC_MSI = 0,
    CLOCK_SRC_HSE,
    CLOCK_SRC_PLL,
    CLOCK_SRC_LSI,
} ClockSource_t;

// Simple status functions
bool HSE_IsReady(void);
bool PLL_IsReady(void);
bool MSI_IsReady(void);
bool LSI_IsReady(void);

// Configuration functions
void RCC_MSI_Config(uint32_t msi_range);
void RCC_HSE_Config(uint32_t hse_freq, bool bypass);
void RCC_PLL_Config(uint32_t source, uint32_t m, uint32_t n, uint32_t p, uint32_t q);
void RCC_SYSCLK_SelectSource(ClockSource_t source);

// Utility functions
uint32_t SystemClock_GetSYSCLKFreq(void);
void SystemClock_PrintConfig(void);

#endif // CLOCKS_H