/**
 * @file app_func_ble.c
 * @brief This file provides BLE management functions
 * @copyright Copyright (c) 2024
 */
#include "app_func_ble.h"
#include "app_config.h"

uint8_t ble_curr_state = BLE_STATE_INVALID;
uint8_t disconnection_reason = 0;

int32_t magnet_rst_ble_def_min_timer = -1;
uint8_t magnet_reset_counter = 0;

bool	ble_whitelist_added = false;
bool	ble_peers_is_deleted = false;

static void app_func_ble_resp_cmd_parser(Cmd_Resp_t resp) {
    uint8_t len_payload_min = 0;
    uint8_t len_payload_max = 0;

    switch(resp.Opcode) {
	case OP_BLE_STAT_GET:
	{
		if(resp.Status == STATUS_SUCCESS) {
			len_payload_min = 1;
			len_payload_max = 2;
			if ((resp.PayloadLen >= len_payload_min) && (resp.PayloadLen <= len_payload_max)) {
				uint8_t ble_state = *(resp.Payload);
				app_func_ble_curr_state_update(ble_state);

				if(resp.PayloadLen == 2U) {
					uint8_t reason = resp.Payload[1];
					app_func_ble_disc_reason_update(reason);
				}
			}
		}
	}
		break;

	case OP_BLE_ADV_START:
	{
		if(resp.Status == STATUS_SUCCESS) {
			len_payload_min = 0;
			len_payload_max = 0;
			if ((resp.PayloadLen >= len_payload_min) && (resp.PayloadLen <= len_payload_max)) {
				app_func_ble_curr_state_update(BLE_STATE_ADV_START);
			}
		}
	}
		break;

	case OP_BLE_ADV_STOP:
	{
		if(resp.Status == STATUS_SUCCESS) {
			len_payload_min = 0;
			len_payload_max = 0;
			if ((resp.PayloadLen >= len_payload_min) && (resp.PayloadLen <= len_payload_max)) {
				app_func_ble_curr_state_update(BLE_STATE_ADV_STOP);
			}
		}
	}
		break;

	case OP_BLE_DISCONNECT:
	{
		if(resp.Status == STATUS_SUCCESS) {
			len_payload_min = 0;
			len_payload_max = 0;
			if ((resp.PayloadLen >= len_payload_min) && (resp.PayloadLen <= len_payload_max)) {
				app_func_ble_curr_state_update(BLE_STATE_ADV_STOP);
			}
		}
	}
		break;

	case OP_BLE_WL_ADD:
	{
		if(resp.Status == STATUS_SUCCESS) {
			len_payload_min = 0;
			len_payload_max = 0;
			if ((resp.PayloadLen >= len_payload_min) && (resp.PayloadLen <= len_payload_max)) {
				ble_whitelist_added = true;
			}
		}
	}
		break;

	case OP_BLE_DEL_PEERS:
	{
		if(resp.Status == STATUS_SUCCESS) {
			len_payload_min = 0;
			len_payload_max = 0;
			if ((resp.PayloadLen >= len_payload_min) && (resp.PayloadLen <= len_payload_max)) {
				ble_peers_is_deleted = true;
			}
		}
	}
		break;

	default:
		break;
    }
}

/**
 * @brief Enable / Disable BLE chip
 * 
 * @param enable Enable / Disable
 */
void app_func_ble_enable(bool enable) {
	ble_curr_state = BLE_STATE_INVALID;
	bsp_wdg_refresh();

	bsp_sp_deinit();
	HAL_GPIO_WritePin(BLE_PWRn_GPIO_Port, BLE_PWRn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_GPIO_WritePin(BLE_RSTn_GPIO_Port, BLE_RSTn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
	HAL_Delay(100);

	if (enable) {
		app_func_command_resp_parser_set(&app_func_ble_resp_cmd_parser);
		HAL_GPIO_WritePin(BLE_RSTn_GPIO_Port, BLE_RSTn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_GPIO_WritePin(BLE_PWRn_GPIO_Port, BLE_PWRn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
		HAL_Delay(100);
		bsp_sp_init(&app_func_command_parser, &bsp_fram_write_cplt_cb);
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(BLE_RDY_GPIO_Port, BLE_RDY_Pin) == GPIO_PIN_RESET) { /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
			bsp_wdg_refresh();
			HAL_GPIO_WritePin(BLE_RSTn_GPIO_Port, BLE_RSTn_Pin, GPIO_PIN_RESET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
			HAL_Delay(100);
			HAL_GPIO_WritePin(BLE_RSTn_GPIO_Port, BLE_RSTn_Pin, GPIO_PIN_SET); /* parasoft-suppress MISRAC2012-RULE_11_4-a "This definition comes from HAL." */
			HAL_Delay(100);
		}

		while(ble_curr_state == BLE_STATE_INVALID) {	//Wait for BLE to be ready
			bsp_wdg_refresh();
			app_func_ble_new_state_get();
			while(!bsp_sp_cmd_handler()) {
				HAL_Delay(1);
			}
		}
	}
}

/**
 * @brief Start BLE advertising
 * 
 * @param p_setting BLE advertising settings
 */
void app_func_ble_adv_start(BLE_ADV_Setting_t* p_setting) {
	Cmd_Req_t cmd = {
			.Opcode = OP_BLE_ADV_START,
			.Payload = (uint8_t*)p_setting,
			.PayloadLen = (uint8_t)sizeof(BLE_ADV_Setting_t),
	};
	app_func_command_req_send(cmd);
}

/**
 * @brief Disconnect BLE connection
 * 
 */
void app_func_ble_disconnect(void) {
	Cmd_Req_t cmd = {
			.Opcode = OP_BLE_DISCONNECT,
			.Payload = NULL,
			.PayloadLen = 0,
	};
	app_func_command_req_send(cmd);
}

/**
 * @brief Update BLE state
 * 
 * @param new_ble_state New BLE state
 */
void app_func_ble_curr_state_update(uint8_t new_ble_state) {
	if (ble_curr_state != new_ble_state) {
		ble_curr_state = new_ble_state;
	}
}

/**
 * @brief Get the current BLE state
 * 
 * @return uint8_t Current BLE state
 */
uint8_t app_func_ble_curr_state_get(void) {
	return ble_curr_state;
}

/**
 * @brief Update the reason for BLE disconnection
 * 
 * @param reason The reason for BLE disconnection
 */
void app_func_ble_disc_reason_update(uint8_t reason) {
	if (reason > 0) {
		disconnection_reason = reason;
	}
}

/**
 * @brief Get the reason for BLE disconnection
 *
 * @return uint8_t The reason for BLE disconnection
 */
uint8_t app_func_ble_disc_reason_get(void) {
	return disconnection_reason;
}

/**
 * @brief Get new BLE state from the BLE chip
 * 
 */
void app_func_ble_new_state_get(void) {
	Cmd_Req_t cmd = {
			.Opcode = OP_BLE_STAT_GET,
			.Payload = NULL,
			.PayloadLen = 0,
	};
	app_func_command_req_send(cmd);
}

/**
 * @brief Get new BLE state from the BLE chip
 *
 */
void app_func_ble_peers_del(void) {
	ble_peers_is_deleted = false;
	Cmd_Req_t cmd = {
			.Opcode = OP_BLE_DEL_PEERS,
			.Payload = NULL,
			.PayloadLen = 0,
	};
	app_func_command_req_send(cmd);
}

/**
 * @brief Callback for BLE dafault timer in sleep or active state
 *
 */
void app_func_ble_default_timer_cb(void) {
	if (magnet_rst_ble_def_min_timer > 0) {
		magnet_rst_ble_def_min_timer--;
	}

	if (magnet_rst_ble_def_min_timer == 0) {
		HAL_ERROR_CHECK(HAL_LPTIM_Counter_Stop_IT(&HANDLE_BLE_DEFAULT_LPTIM));
		magnet_rst_ble_def_min_timer = -1;
		magnet_reset_counter = 0;
	}
}

/**
 * @brief Confirm that BLE is the default.
 *
 * @return bool BLE is default or not.
 */
bool app_func_ble_is_default (void) {
	if ((magnet_rst_ble_def_min_timer > 0) && (magnet_reset_counter >= MAGNET_RESET_COUNT)) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * @brief Callback when magnet lost
 *
 * @param detected_time Detected time
 */
void app_func_ble_magnet_lost_cb(uint8_t detected_time) {
	_Float64 magnet_reset_min_time_f = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_MAGNET_RESET_MIN_TIME, (uint8_t*)&magnet_reset_min_time_f, (uint8_t)sizeof(magnet_reset_min_time_f));
	uint8_t magnet_reset_min_time = (uint8_t)magnet_reset_min_time_f;

	_Float64 magnet_reset_max_time_f = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_MAGNET_RESET_MAX_TIME, (uint8_t*)&magnet_reset_max_time_f, (uint8_t)sizeof(magnet_reset_max_time_f));
	uint8_t magnet_reset_max_time = (uint8_t)magnet_reset_max_time_f;

	if ((detected_time >= magnet_reset_min_time) && (detected_time <= magnet_reset_max_time)) {
		app_func_logs_event_write(EVENT_MAGNET_DETECTION, NULL);
		magnet_reset_counter++;
		if (magnet_reset_counter == 1U) {
			magnet_rst_ble_def_min_timer = MAGNET_RESET_MINUTE_DURATION;
			HAL_ERROR_CHECK(HAL_LPTIM_Counter_Start_IT(&HANDLE_BLE_DEFAULT_LPTIM));
		} else if (magnet_reset_counter == MAGNET_RESET_COUNT) {
			magnet_rst_ble_def_min_timer = BLE_DEFAULT_MINUTE_DURATION;
		}
	}
}
