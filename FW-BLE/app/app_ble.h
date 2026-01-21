/**
 * @file app_ble.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief All Bluetooth applications
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_BLE_H_
#define APP_BLE_H_
#include <stdint.h>

#include "app_ble_adv.h"
#include "nrf_ble_lesc.h"
#include "app_timer.h"

#define APP_BLE_MA_SP_DATA_LEN_MIN            AD_TYPE_MANUF_SPEC_DATA_ID_SIZE + 1
#define APP_BLE_MA_SP_DATA_LEN_MAX            AD_TYPE_MANUF_SPEC_DATA_ID_SIZE + MAX_ADV_MANUF_DATA_DATA_LEN

#define APP_BLE_STATE_MASK_ADV                0b00000001
#define APP_BLE_STATE_MASK_CONNECTED          0b00000010
#define APP_BLE_STATE_MASK_SECURITY           0b00000100
#define APP_BLE_STATE_MASK_NUS_READY          0b00001000

#define APP_BLE_ADDR_TYPE_RANDOM_STATIC_MIN   0b11000000

extern uint16_t* p_conn_handle;
extern app_timer_t* p_sec_check_tmr;

/**
 * @brief Initialization of the Bluetooth application.
 * 
 */
void app_ble_init(void);

/**
 * @brief Disconnect from Bluetooth application.
 *
 */
void app_ble_disconnect(void);

/**
 * @brief Start advertising from bluetooth application.
 * 
 * @param passkey Pointer to 6-digit ASCII string (digit 0..9 only, no NULL termination) passkey to be used during pairing.
 * @param whitelist_enable Enable / disable whitelist
 * @param adv_timeout Timeout of advertising
 * @param passkey_timeout Timeout for entering passkey 
 * @param p_ma_sp_data Manufacturer specific data of advertising data
 * @param data_len Length of manufacturer specific data
 */
void app_ble_start_adv(uint8_t* passkey, bool whitelist_enable, uint32_t adv_timeout, uint32_t passkey_timeout, uint8_t* p_ma_sp_data, uint8_t data_len);

/**
 * @brief Stop advertising from bluetooth application.
 * 
 */
void app_ble_stop_adv(void);

/**
 * @brief Add an address to the whitelist
 * 
 * @param addr Pointer to 48-bit address, LSB format.
 * @param addr_type Address types, see @ref BLE_GAP_ADDR_TYPES. Only BLE_GAP_ADDR_TYPE_PUBLIC or BLE_GAP_ADDR_TYPE_RANDOM_STATIC is valid.
 */
void app_ble_whitelist_add(uint8_t* addr, uint8_t addr_type);

/**
 * @brief Function for deleting all data stored for all peers.
 *
 */
void app_ble_peers_del(void);

/**
 * @brief Send data from bluetooth application.
 * 
 * @param p_data The data to send.
 * @param p_length The length of the data to send.
 * @return uint32_t Error Codes number
 */
uint32_t  app_ble_data_send(uint8_t* p_data, uint16_t* p_length);

/**
 * @brief Get the status of BLE
 * 
 * @return uint8_t* The status of BLE
 */
uint8_t* app_ble_state_get(void);

/**
 * @brief Handler for updating BLE status.
 * 
 */
void app_ble_state_handler(void);

#endif