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
