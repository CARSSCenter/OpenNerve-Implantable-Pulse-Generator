/**
 * @file app_mode_ble_connection.h
 * @brief This file contains all the function prototypes for the app_mode_ble_connection.c file
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_MODE_BLE_CONNECTION_H_
#define INC_APP_MODE_BLE_CONNECTION_H_

/**
 * @brief Handler for BLE connection mode
 *
 */
void app_mode_ble_conn_handler(void);

/**
 * @brief Callback for the timers in BLE connection mode, unit: ms
 *
 */
void app_mode_ble_conn_timer_cb(void);

#endif /* INC_APP_MODE_BLE_CONNECTION_H_ */
