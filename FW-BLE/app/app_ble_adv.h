/**
 * @file app_ble_adv.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of Bluetooth advertising
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_BLE_ADV_H_
#define APP_BLE_ADV_H_
#include "ble_advertising.h"
#include "ble_advdata.h"
#include <stdbool.h>

#define APP_BLE_CONN_CFG_TAG      1 /**< A tag identifying the SoftDevice BLE configuration. */

#define MAX_ADV_MANUF_DATA_DATA_LEN    (31 - AD_TYPE_FLAGS_SIZE - AD_DATA_OFFSET - AD_TYPE_MANUF_SPEC_DATA_ID_SIZE)

extern ble_advertising_t* p_adv;

/**
 * @brief Function for initializing the Advertising functionality.
 * 
 */
void app_ble_adv_init(void);

/**
 * @brief Function for start the Advertising functionality.
 * 
 * @param whitelist_enable Enable / disable whitelist
 * @param adv_timeout Timeout of advertising
 * @param p_ma_sp_data Manufacturer specific data of advertising data
 * @param data_len Length of manufacturer specific data
 */
void app_ble_adv_start(bool whitelist_enable, uint32_t adv_timeout, uint8_t* p_ma_sp_data, uint8_t data_len);

/**
 * @brief Function for stop the Advertising functionality.
 * 
 */
void app_ble_adv_stop(void);

/**
 * @brief Get the status of the advertising
 * 
 * @return true Advertising starts
 * @return false Advertising stops
 */
bool app_ble_adv_state_get(void);

#endif