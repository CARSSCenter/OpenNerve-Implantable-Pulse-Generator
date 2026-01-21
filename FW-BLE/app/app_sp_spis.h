/**
 * @file app_sp_spis.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of SPI slave port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_SP_SPIS_H_
#define APP_SP_SPIS_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Function for initializing the SPI slave.
 * 
 */
void app_sp_spis_init(void);

/**
 * @brief Put data to the buffer of the spis port.
 * 
 * @param data The data to put.
 */
void app_sp_spis_put(uint8_t* data, uint16_t size);

#endif