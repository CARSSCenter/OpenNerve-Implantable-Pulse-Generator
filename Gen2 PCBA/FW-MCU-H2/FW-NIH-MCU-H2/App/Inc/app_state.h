/**
 * @file app_state.h
 * @brief This file contains all the function prototypes for the app_state.c file
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_STATE_H_
#define INC_APP_STATE_H_
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Handler for shutdown state
 *
 */
void app_state_shutdown_handler(void);

/**
 * @brief Handler for sleep state
 *
 */
void app_state_sleep_handler(void);

/**
 * @brief Handler for active state (No operating mode)
 *
 */
void app_state_active_handler(void);

/**
 * @brief Float BATT.SW_EN (hi-Z) so the battery disconnects (BOARD_HAS_BATT_DISCONNECT only)
 *
 */
void app_state_batt_sw_float(void);

/**
 * @brief Drive BATT.SW_EN high to keep the battery connected (BOARD_HAS_BATT_DISCONNECT only)
 *
 */
void app_state_batt_sw_hold(void);

#endif /* INC_APP_STATE_H_ */
