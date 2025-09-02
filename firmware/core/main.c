/******************************************************************************
  * @file    main.c
  * @brief   Autonomiczny węzeł czujnikowy z LoRa, OTA i gatewayem RPi
  * 
  * Cel Projektu:
  * Kompleksowy, end-to-end system IoT obejmujący projekt PCB, firmware 
  * z bootloaderem OTA oraz software gatewaya.
  * 
  * 
  * Topologia Systemu:
  * [Węzeł Czujnikowy (STM32L0/L4 + RFM95)] --(LoRa)--> [Raspberry Pi Gateway]
  * 
  * Sprzęt:
  * - MCU: STM32L476(ultra-low-power)
  * - LoRa: Moduł RFM95/96 (SX1276) w paśmie 868/915 MHz +
  * - Czujniki:
  *   + Ciśnienie: BMP280 (I2C)
  *   + Światło: Fotorezystor (ADC)
  * - Zasilanie: Bateria Li-Ion 18650 z regulatorem step-down (TPS62912/MP2315) 
  *   i ładowarką + panel sloneczny
  * - We/Wy: LED statusowy, przycisk, złącze SWD
  * 
  * 
  * Protokół Komunikacyjny:
  * - LoRa: SF7, 125 kHz BW, CR 4/5 (optymalizacja szybkości/energii)
  * - Ramka danych UART do RPi:
  *   [0x55 0xAA][CMD][LEN][DATA...][CRC8]
  * - Przykładowe komendy: 
  *   0x01 (send data), 0x02 (set interval), 0xA0 (firmware update)
  * 
  * Oprogramowanie:
  * Firmware (STM32):
  * - Niskopoziomowe driversy (UART, I2C, SPI, ADC) z przerwaniami/DMA
  * - Maszyna stanów:
  *   1. INIT_MSI: Inicjalizacja, health check peryferiów (MSI ~4MHz).
  *   2. HIGH_PERF_TX: Przejście na HSE+PLL, inicjacja i transmisja LoRa (oczekiwanie na ACK).
  *   3. SENSOR_READ: Powrót na MSI, pobranie danych z czujnika (UART).
  *   4. GO_TO_SLEEP: Konfiguracja RTC (LSE) i wejście w tryb STOP.
  *   5. ERROR: Awaryjne zatrzymanie systemu (miganie LED) w przypadku błędu.
  * - Bootloader OTA
  * - Metadane: 1 page flash na wersję firmware'u i CRC
  * 
  * Daemon (Raspberry Pi):
  * - Demon w C (systemd service)
  * - Non-blocking odczyt UART z parsowaniem ramek
  * - Zapis do bazy danych SQLite
  * - Logowanie do journalctl
  * 
  * CI/CD (GitHub Actions):
  * - Pipeline firmware: Build (ARM GCC), test, package (.bin)
  * 
  * @author        Mateusz Kozlowski
  * @date          21.08.25
  * @version       v0.1
  * @target        STM32L476
  * @IDE           VSCode + Makefile
  * @repository    https://github.com/Mkozlowsk/bare-metal-iot-weather-node
  *****************************************************************************/


#include "stm32l4xx.h"
#define LED_PIN 5 


void _init(void) {}



void simple_delay(void) {
    for (volatile int i = 0; i < 10000; ++i);
}

int main(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    GPIOA->MODER &= ~(0x3 << (LED_PIN * 2));
    GPIOA->MODER |= (0x1 << (LED_PIN * 2));

    while (1) {
        GPIOA->ODR ^= (1 << LED_PIN); // Toggle LED
        simple_delay();
    }
}