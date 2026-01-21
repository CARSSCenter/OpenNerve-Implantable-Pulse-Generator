/**
 * @file app_sp_uart.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of UART port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_SP_UART_H_
#define APP_SP_UART_H_

#include <stdint.h>

/**
 * @brief Function for initializing the UART module.
 * 
 */
void app_sp_uart_init(void);

/**
 * @brief Put data to the buffer of the UART.
 * 
 * @param data The data to put.
 * @param size The size of the data to put.
 */
void app_sp_uart_put(uint8_t* data, uint16_t size);

#endif