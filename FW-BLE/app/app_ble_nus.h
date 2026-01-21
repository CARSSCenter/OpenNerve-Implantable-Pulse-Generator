/**
 * @file app_ble_nus.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of Bluetooth Nordic UART Service
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_BLE_NUS_H_
#define APP_BLE_NUS_H_

#include <stdbool.h>
#include "ble_nus.h"
#include "nrf_ble_qwr.h"

extern ble_nus_t* p_nus;

/**
 * @brief Function for initializing services that will be used by the application.
 * 
 * @param p_qwr Queued Writes structure. This structure must be
 *              supplied by the application. It is initialized by this function
 *              and is later used to identify the particular Queued Writes instance.
 */
void      app_ble_nus_init(nrf_ble_qwr_t* p_qwr);

/**
 * @brief Function for sending a data to the peer.
 * 
 * @param p_data Data to be sent.
 * @param p_length Pointer Length of the data. Amount of sent bytes.
 * @return uint32_t If the data was sent successfully. Otherwise, an error code is returned.
 */
uint32_t  app_ble_nus_data_send(uint8_t* p_data, uint16_t* p_length);

/**
 * @brief Checks if this service is ready to send.
 * 
 * @return true The service is ready to send.
 * @return false The service is not ready to send.
 */
bool      app_ble_nus_is_ready(void);

#endif