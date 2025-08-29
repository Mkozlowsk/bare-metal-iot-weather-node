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