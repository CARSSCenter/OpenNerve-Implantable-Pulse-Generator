/**
 * @file app_func_state_machine.h
 * @brief This file contains all the function prototypes for the app_func_state_machine.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_STATE_MACHINE_H_
#define FUNCTIONS_INC_APP_FUNC_STATE_MACHINE_H_
#include <stdint.h>
#include <stdbool.h>

#define	STATE_INVALID					0x0000U		/*!< Invalid state */
#define	STATE_SLEEP						0x0100U		/*!< Sleep state */
#define	STATE_ACT						0x0200U		/*!< Active state */
#define	STATE_ACT_MODE_BLE_ACT			0x0201U		/*!< Active state with BLE Active mode */
#define	STATE_ACT_MODE_BLE_CONN			0x0202U		/*!< Active state with BLE Connection mode */
#define	STATE_ACT_MODE_THERAPY_SESSION	0x0203U		/*!< Active state with Therapy Session mode */
#define	STATE_ACT_MODE_IMPED_TEST		0x0204U		/*!< Active state with Impedance Test mode */
#define	STATE_ACT_MODE_BATT_TEST		0x0205U		/*!< Active state with Battery Test mode */
#define	STATE_ACT_MODE_OAD				0x0206U		/*!< Active state with OAD mode */
#define	STATE_ACT_MODE_BSL				0x0207U		/*!< Active state with BSL mode */
#define	STATE_ACT_MODE_DVT				0x0208U		/*!< Active state with DVT mode */
#define	STATE_ACT_MODE_WPT_HIGH			0x0209U		/*!< Active state with WPT charging active */
#define	STATE_ACT_MODE_WPT_PAUSED		0x020AU		/*!< Active state with WPT charging paused */
#define	STATE_SHUTDOWN					0xFFFFU		/*!< Shutdown state */

/**
 * @brief Set the current application state
 *
 * @param state current application state
 */
void app_func_sm_current_state_set(uint16_t state);

/**
 * @brief Get the current application state
 *
 * @return uint16_t current application state
 */
uint16_t app_func_sm_current_state_get(void);

/**
 * @brief Enable timer of impedance test.
 *
 */
void app_func_sm_impedance_timer_enable(void);

/**
 * @brief Enable timer of battery test.
 *
 */
void app_func_sm_battery_timer_enable(void);

/**
 * @brief Initialization of the state machine management
 *
 */
void app_func_sm_init(void);

/**
 * @brief Enable / Disable scheduled therapy session
 *
 * @param enable Enable / Disable
 */
void app_func_sm_schd_therapy_enable(bool enable);

/**
 * @brief Callback for wakeup timers in sleep state
 *
 */
void app_func_sm_wakeup_timer_cb(void);

/**
 * @brief Callback for confirmation timer in sleep state
 *
 */
void app_func_sm_confirmation_timer_cb(void);

/**
 * @brief Callback when magnet lost
 *
 * @param detected_time Detected time
 */
void app_func_sm_magnet_lost_cb(uint8_t detected_time);

/**
 * @brief Callback when VRECT coil is detected or lost
 *
 * @param coil_present true = coil present (VRECT_DETn falling/low), false = coil removed (VRECT_DETn rising/high)
 */
void app_func_sm_vrect_coil_cb(bool coil_present);

#endif /* FUNCTIONS_INC_APP_FUNC_STATE_MACHINE_H_ */
