/**
 * @file app_sp.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of serial port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_SP_H_
#define APP_SP_H_

#include <stdint.h>

#define SP_ALL    0
#define SP_SPI    1
#define SP_UART   2

#define DATA_BUFFER_MAX_SIZE      256
#define DATA_BUFFER_GROUP_COUNT   10

/**
 * @brief Initialization of the serial port.
 * 
 */
void app_sp_init(void);

/**
 * @brief Put data to the buffer of the serial port.
 * 
 * @param port The port to put.
 * @param data The data to put.
 * @param size The size of the data to put.
 */
void app_sp_put(uint8_t port, uint8_t const* data, uint16_t size);

/**
 * @brief Process after receiving data from the buffer of the serial port.
 * 
 * @param port The port on which the data was received.
 * @param data Received data.
 * @param size The size of the received data.
 */
void app_sp_on_rx_data(uint8_t port, uint8_t const* data, uint16_t size);

/**
 * @brief The process after the serial port transaction is ready.
 * 
 * @param port The port that is ready for transmission.
 */
void app_sp_on_tx_ready(uint8_t port);

/**
 * @brief Handler for transferring data from serial port to BLE.
 * 
 */
void app_sp_ble_handler(void);

#endif