/**
 * @file app_func_ble.h
 * @brief This file contains all the function prototypes for the app_func_ble.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_BLE_H_
#define FUNCTIONS_INC_APP_FUNC_BLE_H_
#include <stdint.h>
#include <stdbool.h>

#define	MAGNET_RESET_COUNT				5U		/*!< Count value of BLE reset triggered by magnetic induction */
#define	MAGNET_RESET_MINUTE_DURATION 	10		/*!< The duration of a BLE reset that can be triggered by magnetic induction, unit: minute */
#define	BLE_DEFAULT_MINUTE_DURATION 	10		/*!< The duration of a BLE reset to default settings, unit: minute */

#define	BLE_STATE_INVALID		0b11111111U		/*!< Invalid state of BLE */
#define	BLE_STATE_ADV_STOP		0b00000000U		/*!< BLE advertising stop state */
#define	BLE_STATE_ADV_START		0b00000001U		/*!< BLE advertising start state */
#define	BLE_STATE_CONNECT		0b00000010U		/*!< BLE connection state */
#define	BLE_STATE_SEC_EN		0b00000100U		/*!< BLE security enabled state */
#define	BLE_STATE_NUS_READY		0b00001000U		/*!< BLE NUS ready state */

#define BLE_COMPANY_ID_RESERVED	{0xF0,0xF0}		/*!< Reserved company ID. */
#define BLE_COMPANY_ID_TEST		{0xFF,0xFF}		/*!< For internal testing, not for shipping end product */

#define LEN_BLE_MSD_MAX			24U				/*!< The max length of the Manufacturer Specific Data */

typedef struct {
	uint8_t passkey[6];							/*!< Passkey for BLE pairing */
	uint8_t whitelist_enable;					/*!< Enable whitelist or not */
} BLE_Advance_t;

typedef struct {
	BLE_Advance_t	advance;					/*!< Advanced settings for BLE pairing */
	uint8_t adv_timeout[sizeof(uint32_t)];		/*!< Timeout for BLE advertising */
	uint8_t passkey_timeout[sizeof(uint32_t)];	/*!< Timeout for entering passkey */
	uint8_t companyid[sizeof(uint16_t)];		/*!< Company ID for BLE advertising */
	uint8_t	msd[LEN_BLE_MSD_MAX];				/*!< Manufacturer Specific Data for BLE advertising */
} BLE_ADV_Setting_t;

/**
 * @brief Enable / Disable BLE chip
 * 
 * @param enable Enable / Disable
 */
void app_func_ble_enable(bool enable);

/**
 * @brief Start BLE advertising
 * 
 * @param p_setting BLE advertising settings
 */
void app_func_ble_adv_start(BLE_ADV_Setting_t* p_setting);

/**
 * @brief Disconnect BLE connection
 * 
 */
void app_func_ble_disconnect(void);

/**
 * @brief Update BLE state
 * 
 * @param new_ble_state New BLE state
 */
void app_func_ble_curr_state_update(uint8_t new_ble_state);

/**
 * @brief Get the current BLE state
 * 
 * @return uint8_t Current BLE state
 */
uint8_t app_func_ble_curr_state_get(void);

/**
 * @brief Update the reason for BLE disconnection
 * 
 * @param reason The reason for BLE disconnection
 */
void app_func_ble_disc_reason_update(uint8_t reason);

/**
 * @brief Get the reason for BLE disconnection
 *
 * @return uint8_t The reason for BLE disconnection
 */
uint8_t app_func_ble_disc_reason_get(void);

/**
 * @brief Get new BLE state from the BLE chip
 * 
 */
void app_func_ble_new_state_get(void);

/**
 * @brief Get new BLE state from the BLE chip
 *
 */
void app_func_ble_peers_del(void);

/**
 * @brief Callback for BLE dafault timer in sleep or active state
 *
 */
void app_func_ble_default_timer_cb(void);

/**
 * @brief Confirm that BLE is the default.
 *
 * @return bool BLE is default or not.
 */
bool app_func_ble_is_default (void);

/**
 * @brief Callback when magnet lost
 *
 * @param detected_time Detected time
 */
void app_func_ble_magnet_lost_cb(uint8_t detected_time);

#endif /* FUNCTIONS_INC_APP_FUNC_BLE_H_ */
