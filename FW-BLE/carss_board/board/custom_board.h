#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

#define BLE_SCK_PIN                 3     //PA5, SPI1.SCK
#define BLE_MOSI_PIN                5     //PA7, SPI1.MOSI
#define BLE_MISO_PIN                4     //PA6, SPI1.MISO
#define BLE_CSn_PIN                 8     //PG5, SPI1.BLE_CSn
#define BLE_RDY_PIN                 17    //PG12, BLE_P.1 
#define BLE_REQ_PIN                 15    //PG4, BLE_P.2

#define BLE_RX_PIN                  18
#define BLE_TX_PIN                  20

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_BOARD_H