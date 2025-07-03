/**
 * @file app_mode_ble_active.c
 * @brief This file provides management of the BLE active mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_ble_active.h"
#include "app_config.h"

static uint32_t adv_ms_timer = 0;
static uint8_t ble_act_user_class = USER_CLASS_INVALID;
static bool active_disconnect = false;

/**
 * @brief Parser for request commands in BLE active mode, used to communicate with the remote end
 * 
 * @param req Request command to be parsed
 * @return Cmd_Resp_t The response command to be replied after parsing the request command
 */
static Cmd_Resp_t app_mode_ble_act_cmd_parser(Cmd_Req_t req) {
	Cmd_Resp_t resp = {
			.Opcode 		= req.Opcode,
			.Status 		= STATUS_SUCCESS,
			.Payload 		= NULL,
			.PayloadLen 	= 0,
	};

	switch(req.Opcode) {
	case OP_AUTH:
	{
		uint8_t datalen_ipg_fw_version 	= app_func_para_datalen_get((const uint8_t*)HPID_IPG_FW_VERSION);
		uint8_t datalen_ble_id 			= app_func_para_datalen_get((const uint8_t*)HPID_IPG_BLE_ID);
		uint8_t len_payload_min = (uint8_t)sizeof(ECDSA_Data_t) + datalen_ipg_fw_version + datalen_ble_id;
		uint8_t len_payload_max = (uint8_t)sizeof(ECDSA_Data_t) + datalen_ipg_fw_version + datalen_ble_id;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			ECDSA_Data_t ecdsa_data = {0};
			(void)memcpy((uint8_t*)&ecdsa_data, req.Payload, sizeof(ECDSA_Data_t));
			ble_act_user_class = app_func_auth_user_class_get(ecdsa_data);

			if (ble_act_user_class == USER_CLASS_INVALID) {
				resp.Status = STATUS_INVALID;
				active_disconnect = true;
			}
			else {
				if (ble_act_user_class == USER_CLASS_CLINICIAN || ble_act_user_class == USER_CLASS_PATIENT) {
					app_func_para_data_set((const uint8_t*)HPID_LINKED_PRC_FW_VERSION, 	&req.Payload[sizeof(ECDSA_Data_t)]);
					app_func_para_data_set((const uint8_t*)HPID_LINKED_PRC_BLE_ID, 		&req.Payload[sizeof(ECDSA_Data_t) + datalen_ipg_fw_version]);
				}
				app_func_sm_current_state_set(STATE_ACT_MODE_BLE_CONN);
			}
			resp.PayloadLen = (uint8_t)sizeof(uint8_t);
			resp.Payload = &ble_act_user_class;
		}
	}
		break;

	default:
	{
		resp.Status = STATUS_OPCODE_ERR;
	}
		break;
	}

	return resp;
}

/**
 * @brief Get the current user class
 * 
 * @return uint8_t The current user class
 */
uint8_t app_mode_ble_act_userclass_get(void) {
	return ble_act_user_class;
}

/**
 * @brief Handler for BLE active mode
 * 
 */
void app_mode_ble_act_handler(void) {
	uint16_t curr_state = app_func_sm_current_state_get();
	app_func_command_req_parser_set(&app_mode_ble_act_cmd_parser);
	ble_act_user_class = USER_CLASS_INVALID;

	BLE_ADV_Setting_t setting;
	app_func_para_data_get((const uint8_t*)BPID_BLE_PASSKEY, setting.advance.passkey, (uint8_t)sizeof(setting.advance.passkey));
	app_func_para_data_get((const uint8_t*)BPID_BLE_WHITELIST, &setting.advance.whitelist_enable, (uint8_t)sizeof(setting.advance.whitelist_enable));

	_Float64 adv_timeout_f = 0.0;
	app_func_para_data_get((const uint8_t*)HPID_BLE_BROADCAST_TIMEOUT, (uint8_t*)&adv_timeout_f, (uint8_t)sizeof(adv_timeout_f));
	uint32_t adv_timeout = (uint32_t)adv_timeout_f;
	(void)memcpy((uint8_t*)setting.adv_timeout, (uint8_t*)&adv_timeout, sizeof(uint32_t));

	uint32_t passkey_timeout = adv_timeout / 2U;
	(void)memcpy((uint8_t*)setting.passkey_timeout, (uint8_t*)&passkey_timeout, sizeof(uint32_t));

	app_func_para_data_get((const uint8_t*)BPID_BLE_COMPANY_ID, setting.companyid, (uint8_t)sizeof(setting.companyid));
	app_func_para_data_get((const uint8_t*)HPID_IPG_BLE_ID, setting.bleid, (uint8_t)sizeof(setting.bleid));

	if (app_func_ble_is_default()) {
		app_func_para_defdata_get((const uint8_t*)BPID_BLE_PASSKEY, (uint8_t*)setting.advance.passkey);
		setting.advance.whitelist_enable = false;
	}

	app_func_ble_enable(true);
	uint8_t curr_ble_state = app_func_ble_curr_state_get();

	uint8_t bleidlen = app_func_para_datalen_get((const uint8_t*)HPID_IPG_BLE_ID);
	if (bleidlen > LEN_BLE_ID_MAX) {
		bleidlen = LEN_BLE_ID_MAX;
	}

	adv_ms_timer = adv_timeout * 1000U;
	app_func_ble_adv_start(&setting, bleidlen);
	while(!bsp_sp_cmd_handler()) {
		HAL_Delay(1);
	}
	while((curr_ble_state != BLE_STATE_ADV_START) && (adv_ms_timer > 0U)) {
		bsp_wdg_refresh();
		app_func_ble_new_state_get();
		while(!bsp_sp_cmd_handler()) {
			HAL_Delay(1);
		}
		curr_ble_state = app_func_ble_curr_state_get();
	}

	while(curr_state == STATE_ACT_MODE_BLE_ACT) {
		bsp_wdg_refresh();
		if (adv_ms_timer == 0U) {
			app_func_sm_current_state_set(STATE_ACT);
			app_func_ble_enable(false);
		}
		else {
			if (active_disconnect) {
				app_func_ble_disconnect();
				active_disconnect = false;
			}
			else {
				app_func_ble_new_state_get();
			}
			bsp_sp_cmd_handler();
			HAL_Delay(50);
			curr_ble_state = app_func_ble_curr_state_get();
			if (curr_ble_state == BLE_STATE_ADV_STOP) {
				app_func_ble_adv_start(&setting, bleidlen);
				while(!bsp_sp_cmd_handler()) {
					HAL_Delay(1);
				}
				while((curr_ble_state != BLE_STATE_ADV_START) && (adv_ms_timer > 0U)) {
					bsp_wdg_refresh();
					app_func_ble_new_state_get();
					while(!bsp_sp_cmd_handler()) {
						HAL_Delay(1);
					}
					curr_ble_state = app_func_ble_curr_state_get();
				}
			}
		}
		curr_state = app_func_sm_current_state_get();
	}
}

/**
 * @brief Callback for the timers in BLE active mode, unit: ms
 * 
 */
void app_mode_ble_act_timer_cb(void) {
	if (adv_ms_timer > 0U) {
		adv_ms_timer--;
	}
}
