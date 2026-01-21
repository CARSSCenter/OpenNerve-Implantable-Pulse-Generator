/**
 * @file app_mode_ble_active.c
 * @brief This file provides management of the BLE active mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_ble_active.h"
#include "app_config.h"

const uint32_t msd_update_interval_s = 1U;

static uint32_t adv_ms_timer = 0U;
static uint8_t ble_act_user_class = USER_CLASS_INVALID;
static bool active_disconnect = false;

/**
 * @brief Set or clear a bit in a buffer based on a GPIO pin state.
 *
 * @param buffer       Pointer to the target buffer where the bit will be modified.
 * @param buffer_size  Size of the buffer in bytes. Used to validate @p bit_index.
 * @param bit_index    Index of the bit to update (0 = least significant bit of buffer[0]).
 * @param GPIOx        Pointer to the GPIO port (e.g., GPIOA, GPIOB).
 * @param GPIO_Pin     GPIO pin number (e.g., GPIO_PIN_0).
 *
 * @return uint8_t HAL status
 */
static HAL_StatusTypeDef setBitFromGpioState(uint8_t *buffer, size_t buffer_size, size_t bit_index, const GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (!buffer || !GPIOx) {
        return HAL_ERROR; // Null pointer error
    }

    size_t byte_index = bit_index / 8; // Calculate the byte index
    size_t bit_position = bit_index % 8; // Calculate the bit position within the byte

    if (byte_index >= buffer_size) {
        return HAL_ERROR; // Out of range error
    }

    GPIO_PinState state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin); // Read the GPIO pin state

    if (state == GPIO_PIN_SET) {
        buffer[byte_index] |= (1 << bit_position); // Set the bit to 1
    } else {
        buffer[byte_index] &= ~(1 << bit_position); // Clear the bit to 0
    }

    return HAL_OK; // Success
}

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
		resp.Status = STATUS_USER_CLASS_ERR;
	}
		break;
	}

	return resp;
}

/**
 * @brief Update the Manufacturer Specific Data (MSD) field of a BLE advertising packet.
 *
 * @param p_msd Pointer to the msd field of BLE advertising settings structure
 *
 * @return uint8_t 	The number of used bytes in the MSD buffer
 */
uint8_t app_mode_ble_act_adv_msd_update(uint8_t* p_msd) {
	static uint16_t dvdd_div4;
	static uint16_t batt[2];
	static uint16_t imp[2];
	static uint16_t threm[3];

	bsp_adc_single_sampling(HANDLE_ID_ADC1, ADC1_CHANNEL_DVDD, &dvdd_div4, 1, 1000);
	app_func_meas_batt_mon_meas(&batt[0], &batt[1]);
	bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_IMP_INA, &imp[0], 1, 1000);
	bsp_adc_single_sampling(HANDLE_ID_ADC4, ADC4_CHANNEL_IMP_INB, &imp[1], 1, 1000);
	app_func_meas_therm_meas(THERM_ID_REF, (uint8_t*)&threm[0], sizeof(uint16_t), 1000);
	app_func_meas_therm_meas(THERM_ID_OUT, (uint8_t*)&threm[1], sizeof(uint16_t), 1000);
	app_func_meas_therm_meas(THERM_ID_OFST, (uint8_t*)&threm[2], sizeof(uint16_t), 1000);

	uint8_t dvdd_100mv = dvdd_div4 * 4 / 100;
	uint8_t battA_100mv = batt[0] / 100;
	uint8_t battB_100mv = batt[1] / 100;
	uint8_t impA_10mv = imp[0] / 10;
	uint8_t impB_10mv = imp[1] / 10;
	uint8_t thremRef_10mv = threm[0] / 10;
	uint8_t thremOut_10mv = threm[1] / 10;
	uint8_t thremOfst_10mv = threm[2] / 10;

	uint8_t* buff_offset = p_msd;
	*buff_offset++ = dvdd_100mv;    // msd[0]
	*buff_offset++ = battA_100mv;   // msd[1]
	*buff_offset++ = battB_100mv;   // msd[2]
	*buff_offset++ = impA_10mv;     // msd[3]
	*buff_offset++ = impB_10mv;     // msd[4]
	*buff_offset++ = thremRef_10mv; // msd[5]
	*buff_offset++ = thremOut_10mv; // msd[6]
	*buff_offset++ = thremOfst_10mv;// msd[7]

	uint8_t buff_freesize = LEN_BLE_MSD_MAX - ((uint8_t)(buff_offset - p_msd));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 0, 	VRECT_DETn_GPIO_Port, VRECT_DETn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 1, 	VRECT_OVPn_GPIO_Port, VRECT_OVPn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 2, 	VCHG_PGOOD_GPIO_Port, VCHG_PGOOD_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 3, 	CHG1_STATUS_GPIO_Port, CHG1_STATUS_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 4, 	CHG1_OVP_ERRn_GPIO_Port, CHG1_OVP_ERRn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 5, 	CHG2_STATUS_GPIO_Port, CHG2_STATUS_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 6, 	CHG2_OVP_ERRn_GPIO_Port, CHG2_OVP_ERRn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 7, 	ENG2_SDNn_GPIO_Port, ENG2_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 8, 	ENG1_SDNn_GPIO_Port, ENG1_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 9, 	ECG_RLD_GPIO_Port, ECG_RLD_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 10, ECG_HR_SDNn_GPIO_Port, ECG_HR_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 11, ECG_RR_SDNn_GPIO_Port, ECG_RR_SDNn_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 12, TEMP_EN_GPIO_Port, TEMP_EN_Pin));
	HAL_ERROR_CHECK(setBitFromGpioState(buff_offset, buff_freesize, 13, IMP_EN_GPIO_Port, IMP_EN_Pin));

	return ((uint8_t)(buff_offset - p_msd));
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

	BLE_ADV_Setting_t setting = {0};
	_Float64 adv_timeout_f = 0.0;

	uint8_t bleid_len = app_func_para_datalen_get((const uint8_t*)HPID_IPG_BLE_ID);
	uint8_t offset = app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);

	app_func_para_data_get((const uint8_t*)BPID_BLE_PASSKEY, setting.advance.passkey, (uint8_t)sizeof(setting.advance.passkey));
	app_func_para_data_get((const uint8_t*)BPID_BLE_WHITELIST, &setting.advance.whitelist_enable, (uint8_t)sizeof(setting.advance.whitelist_enable));
	app_func_para_data_get((const uint8_t*)HPID_BLE_BROADCAST_TIMEOUT, (uint8_t*)&adv_timeout_f, (uint8_t)sizeof(adv_timeout_f));
	app_func_para_data_get((const uint8_t*)HPID_IPG_BLE_ID, (uint8_t*)(&setting.msd[offset]), bleid_len);
	app_func_para_data_get((const uint8_t*)BPID_BLE_COMPANY_ID, setting.companyid, (uint8_t)sizeof(setting.companyid));

	uint32_t adv_timeout = (uint32_t)adv_timeout_f;
	uint32_t passkey_timeout = adv_timeout / 2U;

	(void)memcpy((uint8_t*)setting.passkey_timeout, (uint8_t*)&passkey_timeout, sizeof(uint32_t));
	(void)memcpy((uint8_t*)setting.adv_timeout, (uint8_t*)&msd_update_interval_s, sizeof(uint32_t));

	setting.msd[LEN_BLE_MSD_MAX - 1] = HW_VERSION;

	if (app_func_ble_is_default()) {
		app_func_para_defdata_get((const uint8_t*)BPID_BLE_PASSKEY, (uint8_t*)setting.advance.passkey);
		setting.advance.whitelist_enable = false;
	}

	app_func_ble_enable(true);
	uint8_t curr_ble_state = app_func_ble_curr_state_get();
	app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);
	app_func_ble_adv_start(&setting);
	while(!bsp_sp_cmd_handler()) {
		HAL_Delay(1);
	}
	adv_ms_timer = adv_timeout * 1000U;
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
				HAL_Delay(50);
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
				app_mode_ble_act_adv_msd_update((uint8_t*)setting.msd);
				app_func_ble_adv_start(&setting);
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
