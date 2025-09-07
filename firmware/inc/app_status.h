/******************************************************************************
  * @file          app_status.h
  * @brief         Naglowek do kontrolowania statusu aplikacji
  * @author        Mateusz Kozlowski
  * @date          01.09.2025
  * @version       v1.0
  *****************************************************************************/



#ifndef APP_STATUS_H
#define APP_STATUS_H

/**
  * @brief Kody statusy returnowane przez funkcje aplikacji
  */

typedef enum {
    APP_OK       = 0x00U, /**<  Operacja sie powiodla */
    APP_ERROR    = 0x01U, /**< Generic blad */
    APP_TIMEOUT  = 0x02U, /**< Wystapil timeout */
    APP_BUSY     = 0x03U, /**<  Peryferium jest zajete */
    APP_INVALID_PARAM = 0x04U, /**< Zly parametr */
    APP_NOT_READY = 0x05U, /**< Peryferium nie jest gotowe */
    APP_CLOCK_ERROR = 0x06U, /**< Blad konfiguracji zegara */
    APP_ALREADY_ACQUIRED = 0x07U
} App_StatusTypeDef;

#endif // APP_STATUS_H