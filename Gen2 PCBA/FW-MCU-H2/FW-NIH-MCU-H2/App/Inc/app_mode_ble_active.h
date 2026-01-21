/**
 * @file app_mode_ble_active.h
 * @brief This file contains all the function prototypes for the app_mode_ble_active.c file
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_MODE_BLE_ACTIVE_H_
#define INC_APP_MODE_BLE_ACTIVE_H_
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Update the Manufacturer Specific Data (MSD) field of a BLE advertising packet.
 *
 * @param p_msd Pointer to the msd field of BLE advertising settings structure
 *
 * @return uint8_t 	The number of used bytes in the MSD buffer
 */
uint8_t app_mode_ble_act_adv_msd_update(uint8_t* p_msd);

/**
 * @brief Get the current user class
 *
 * @return uint8_t The current user class
 */
uint8_t app_mode_ble_act_userclass_get(void);

/**
 * @brief Handler for BLE active mode
 *
 */
void app_mode_ble_act_handler(void);

/**
 * @brief Callback for the timers in BLE active mode, unit: ms
 *
 */
void app_mode_ble_act_timer_cb(void);

#endif /* INC_APP_MODE_BLE_ACTIVE_H_ */
