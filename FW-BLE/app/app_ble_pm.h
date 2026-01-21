/**
 * @file app_ble_pm.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications for the peer manager
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_BLE_PM_H_
#define APP_BLE_PM_H_
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Function for initializing the Peer Manager.
 * 
 */
void app_ble_pm_init(void);

/**
 * @brief Set the passkey to be used during pairing.
 * 
 * @param passkey Pointer to 6-digit ASCII string (digit 0..9 only, no NULL termination) passkey to be used during pairing.
 */
void app_ble_pm_passkey_set(uint8_t* passkey);

/**
 * @brief Add an address to the whitelist
 * 
 * @param addr Pointer to 48-bit address, LSB format.
 * @param addr_type Address types, see @ref BLE_GAP_ADDR_TYPES. Only BLE_GAP_ADDR_TYPE_PUBLIC or BLE_GAP_ADDR_TYPE_RANDOM_STATIC is valid.
 */
void app_ble_pm_whitelist_add(uint8_t* addr, uint8_t addr_type);

/**
 * @brief Function for deleting all data stored for all peers.
 *
 */
void app_ble_pm_peers_del(void);

/**
 * @brief Get the status of security.
 * 
 * @return true Security is enabled.
 * @return false Security is not enabled.
 */
bool app_ble_pm_sec_state_get(void);

#endif