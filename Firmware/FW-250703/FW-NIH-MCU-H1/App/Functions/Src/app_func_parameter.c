/**
 * @file app_func_parameter.c
 * @brief This file provides parameter management functions
 * @copyright Copyright (c) 2024
 */
#include "app_func_parameter.h"
#include "app_config.h"

#define BYTE_PER_ADDRESS		8U		/*!< The data size of each address in the EEPROM */

static uint16_t def_sample_id = 0x0000;
static Parameter_Format_Rawdata_t format_sample_id = {
		.data_def = (uint8_t*)&def_sample_id,
		.data_len = (uint8_t)sizeof(def_sample_id),
};

static uint8_t def_ble_passkey[6] = "000000";
static Parameter_Format_Rawdata_t format_ble_passkey = {
		.data_def = def_ble_passkey,
		.data_len = (uint8_t)sizeof(def_ble_passkey),
};

static uint8_t def_ble_whitelist[1] = { 0x01 };
static Parameter_Format_Rawdata_t format_ble_whitelist = {
		.data_def = def_ble_whitelist,
		.data_len = (uint8_t)sizeof(def_ble_whitelist),
};

static uint8_t def_ble_company_id[2] = BLE_COMPANY_ID_RESERVED;
static Parameter_Format_Rawdata_t format_ble_company_id = {
		.data_def = def_ble_company_id,
		.data_len = (uint8_t)sizeof(def_ble_company_id),
};

static uint8_t def_ipg_serial_number[8] = "YYLL0001";
static Parameter_Format_Rawdata_t format_ipg_serial_number = {
		.data_def = def_ipg_serial_number,
		.data_len = (uint8_t)sizeof(def_ipg_serial_number),
};

static uint8_t def_ipg_model[6] = "CAR-01";
static Parameter_Format_Rawdata_t format_ipg_model = {
		.data_def = def_ipg_model,
		.data_len = (uint8_t)sizeof(def_ipg_model),
};

static uint8_t def_ipg_ble_id[LEN_BLE_ID_MAX] = {0};
static Parameter_Format_Rawdata_t format_ipg_ble_id = {
		.data_def = def_ipg_ble_id,
		.data_len = (uint8_t)sizeof(def_ipg_ble_id),
};

static uint8_t def_ipg_prod_loc[10] = {0};
static Parameter_Format_Rawdata_t format_ipg_prod_loc = {
		.data_def = def_ipg_prod_loc,
		.data_len = (uint8_t)sizeof(def_ipg_prod_loc),
};

static uint8_t def_ipg_fw_version[6] = APP_FW_VER_STR;
static Parameter_Format_Rawdata_t format_ipg_fw_version = {
		.data_def = def_ipg_fw_version,
		.data_len = (uint8_t)sizeof(def_ipg_fw_version),
};

static uint8_t def_link_fw_version[6]	= "1.0.00";
static Parameter_Format_Rawdata_t format_link_fw_version = {
		.data_def = def_link_fw_version,
		.data_len = (uint8_t)sizeof(def_link_fw_version),
};

static uint8_t def_ipg_manuf_date[10] = "YYYY-MM-DD";
static Parameter_Format_Rawdata_t format_ipg_manuf_date = {
		.data_def = def_ipg_manuf_date,
		.data_len = (uint8_t)sizeof(def_ipg_manuf_date),
};

static uint8_t def_ipg_impla_date[10] = "YYYY-MM-DD";
static Parameter_Format_Rawdata_t format_ipg_impla_date = {
		.data_def = def_ipg_impla_date,
		.data_len = (uint8_t)sizeof(def_ipg_impla_date),
};

static ECC_PublicKey_t PublicKey_Clinician = { APP_ECC_PUBKEY_QX_CLINICIAN, APP_ECC_PUBKEY_QY_CLINICIAN };
static Parameter_Format_Rawdata_t format_PublicKey_Clinician = {
		.data_def = (uint8_t*)&PublicKey_Clinician,
		.data_len = (uint8_t)sizeof(PublicKey_Clinician),
};

static ECC_PublicKey_t PublicKey_Patient = { APP_ECC_PUBKEY_QX_PATIENT, APP_ECC_PUBKEY_QY_PATIENT };
static Parameter_Format_Rawdata_t format_PublicKey_Patient = {
		.data_def = (uint8_t*)&PublicKey_Patient,
		.data_len = (uint8_t)sizeof(PublicKey_Patient),
};

static Parameter_Format_Value_t format_ble_broadcast_timeout 	= {10.0, 	600.0, 	60.0, 	10.0};
static Parameter_Format_Value_t format_ble_idle_connection 		= {10.0, 	600.0, 	300.0, 	10.0};
static Parameter_Format_Value_t format_ble_disconnect_request 	= {10.0, 	600.0, 	10.0, 	10.0};
static Parameter_Format_Value_t format_ble_interval 			= {1.0, 	600.0, 	60.0, 	1.0};
static Parameter_Format_Value_t format_impedance_test_interval 	= {1.0, 	96.0, 	24.0, 	1.0};
static Parameter_Format_Value_t format_battery_test_interval 	= {1.0, 	96.0, 	24.0, 	1.0};
static Parameter_Format_Value_t format_magnet_wakeup_min_time 	= {1.0, 	60.0, 	2.0, 	1.0};
static Parameter_Format_Value_t format_magnet_wakeup_max_time 	= {1.0, 	60.0, 	6.0, 	1.0};
static Parameter_Format_Value_t format_magnet_reset_min_time 	= {1.0, 	60.0, 	9.0, 	1.0};
static Parameter_Format_Value_t format_magnet_reset_max_time 	= {1.0, 	60.0, 	13.0, 	1.0};
static Parameter_Format_Value_t format_battery_er_level 		= {2.0, 	4.0, 	2.6, 	0.1};
static Parameter_Format_Value_t format_battery_eos_level 		= {2.0, 	4.0, 	2.5, 	0.1};
static Parameter_Format_Value_t format_language 				= {1.0, 	4.0, 	1.0, 	1.0};
static Parameter_Format_Value_t format_idle_duration 			= {1.0, 	600.0, 	60.0, 	1.0};
static Parameter_Format_Value_t format_rtc_interrupt_period 	= {60.0, 	60.0, 	60.0, 	1.0};

static Parameter_Format_Value_t format_num_of_therapy_sessions	= {1.0, 	6.0, 	3.0, 	1.0};
static Parameter_Format_Value_t format_pulse_amplitude			= {0.2, 	5.0, 	0.2, 	0.1};
static Parameter_Format_Value_t format_pulse_width				= {100.0, 	1000.0,	500.0, 	50.0};
static Parameter_Format_Value_t format_pulse_frequency			= {1.0, 	100.0, 	5.0, 	1.0};
static Parameter_Format_Value_t format_ramp_up_duration			= {0.0, 	10.0, 	0.0, 	1.0};
static Parameter_Format_Value_t format_ramp_down_duration		= {0.0, 	10.0, 	0.0, 	1.0};
static Parameter_Format_Value_t format_train_on_duration		= {10.0, 	300.0, 	10.0, 	10.0};
static Parameter_Format_Value_t format_train_off_duration		= {0.0, 	300.0, 	90.0, 	10.0};
static Parameter_Format_Value_t format_cathode_electrode_number	= {1.0, 	9.0, 	1.0, 	1.0};
static Parameter_Format_Value_t format_anode_electrode_number	= {1.0, 	9.0, 	2.0, 	1.0};
static Parameter_Format_Value_t format_max_safe_amplitude		= {0.5, 	5.0, 	0.5, 	0.1};
static Parameter_Format_Value_t format_min_safe_impedance		= {100.0, 	600.0, 	450.0, 	10.0};

static Parameter_Format_Value_t format_therapy_session_1_start	= {0.0, 	1438.0, 480.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_1_stop	= {1.0, 	1439.0, 510.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_2_start	= {0.0, 	1438.0, 480.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_2_stop	= {1.0, 	1439.0, 510.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_3_start	= {0.0, 	1438.0, 480.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_3_stop	= {1.0, 	1439.0, 510.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_4_start	= {0.0, 	1438.0, 480.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_4_stop	= {1.0, 	1439.0, 510.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_5_start	= {0.0, 	1438.0, 480.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_5_stop	= {1.0, 	1439.0, 510.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_6_start	= {0.0, 	1438.0, 480.0, 	1.0};
static Parameter_Format_Value_t format_therapy_session_6_stop	= {1.0, 	1439.0, 510.0, 	1.0};

Parameter_t parameters_list[] = {
		{TPID_SAMPLE_ID, 					0U, {&format_sample_id, 			NULL}},

		{BPID_BLE_PASSKEY, 					0U, {&format_ble_passkey, 			NULL}},
		{BPID_BLE_WHITELIST, 				0U, {&format_ble_whitelist, 		NULL}},
		{BPID_BLE_COMPANY_ID, 				0U, {&format_ble_company_id, 		NULL}},

		{HPID_IPG_SERIAL_NUMBER, 			0U,	{&format_ipg_serial_number, 	NULL}},
		{HPID_IPG_MODEL, 					0U,	{&format_ipg_model, 			NULL}},
		{HPID_IPG_BLE_ID, 					0U,	{&format_ipg_ble_id, 			NULL}},
		{HPID_IPG_PRODUCTION_LOCATION, 		0U,	{&format_ipg_prod_loc, 			NULL}},
		{HPID_IPG_FW_VERSION, 				0U,	{&format_ipg_fw_version, 		NULL}},
		{HPID_IPG_MANUFACTURING_DATE, 		0U,	{&format_ipg_manuf_date, 		NULL}},
		{HPID_IPG_IMPLANTATION_DATE, 		0U,	{&format_ipg_impla_date, 		NULL}},
		{HPID_LINKED_CRC_BLE_ID, 			0U,	{&format_ipg_ble_id, 			NULL}},
		{HPID_LINKED_CRC_KEY, 				0U,	{&format_PublicKey_Clinician, 	NULL}},
		{HPID_LINKED_CRC_FW_VERSION, 		0U,	{&format_link_fw_version, 		NULL}},
		{HPID_LINKED_PRC_BLE_ID, 			0U,	{&format_ipg_ble_id, 			NULL}},
		{HPID_LINKED_PRC_KEY, 				0U,	{&format_PublicKey_Patient, 	NULL}},
		{HPID_LINKED_PRC_FW_VERSION, 		0U,	{&format_link_fw_version, 		NULL}},

		{HPID_BLE_BROADCAST_TIMEOUT, 		0U,	{NULL, &format_ble_broadcast_timeout}},
		{HPID_BLE_IDLE_CONNECTION, 			0U,	{NULL, &format_ble_idle_connection}},
		{HPID_BLE_DISCONNECT_REQUEST, 		0U,	{NULL, &format_ble_disconnect_request}},
		{HPID_BLE_INTERVAL, 				0U,	{NULL, &format_ble_interval}},
		{HPID_IMPEDANCE_TEST_INTERVAL, 		0U,	{NULL, &format_impedance_test_interval}},
		{HPID_BATTERY_TEST_INTERVAL, 		0U,	{NULL, &format_battery_test_interval}},
		{HPID_MAGNET_WAKEUP_MIN_TIME, 		0U,	{NULL, &format_magnet_wakeup_min_time}},
		{HPID_MAGNET_WAKEUP_MAX_TIME, 		0U,	{NULL, &format_magnet_wakeup_max_time}},
		{HPID_MAGNET_RESET_MIN_TIME, 		0U,	{NULL, &format_magnet_reset_min_time}},
		{HPID_MAGNET_RESET_MAX_TIME, 		0U,	{NULL, &format_magnet_reset_max_time}},
		{HPID_BATTERY_ER_LEVEL, 			0U,	{NULL, &format_battery_er_level}},
		{HPID_BATTERY_EOS_LEVEL, 			0U,	{NULL, &format_battery_eos_level}},
		{HPID_LANGUAGE, 					0U,	{NULL, &format_language}},
		{HPID_IDLE_DURATION, 				0U,	{NULL, &format_idle_duration}},
		{HPID_RTC_INTERRUPT_PERIOD, 		0U,	{NULL, &format_rtc_interrupt_period}},

		{SPID_NUMBER_OF_THERAPY_SESSIONS,	0U,	{NULL, &format_num_of_therapy_sessions}},
		{SPID_PULSE_AMPLITUDE,				0U,	{NULL, &format_pulse_amplitude}},
		{SPID_PULSE_WIDTH,					0U,	{NULL, &format_pulse_width}},
		{SPID_PULSE_FREQUENCY,				0U,	{NULL, &format_pulse_frequency}},
		{SPID_RAMP_UP_DURATION,				0U,	{NULL, &format_ramp_up_duration}},
		{SPID_RAMP_DOWN_DURATION,			0U,	{NULL, &format_ramp_down_duration}},
		{SPID_TRAIN_ON_DURATION,			0U,	{NULL, &format_train_on_duration}},
		{SPID_TRAIN_OFF_DURATION,			0U,	{NULL, &format_train_off_duration}},
		{SPID_CATHODE_ELECTRODE_NUMBER,		0U,	{NULL, &format_cathode_electrode_number}},
		{SPID_ANODE_ELECTRODE_NUMBER,		0U,	{NULL, &format_anode_electrode_number}},
		{SPID_MAX_SAFE_AMPLITUDE,			0U,	{NULL, &format_max_safe_amplitude}},
		{SPID_MIN_SAFE_IMPEDANCE,			0U,	{NULL, &format_min_safe_impedance}},

		{SPID_THERAPY_SESSION_1_START,		0U,	{NULL, &format_therapy_session_1_start}},
		{SPID_THERAPY_SESSION_1_STOP,		0U,	{NULL, &format_therapy_session_1_stop}},
		{SPID_THERAPY_SESSION_2_START,		0U,	{NULL, &format_therapy_session_2_start}},
		{SPID_THERAPY_SESSION_2_STOP,		0U,	{NULL, &format_therapy_session_2_stop}},
		{SPID_THERAPY_SESSION_3_START,		0U,	{NULL, &format_therapy_session_3_start}},
		{SPID_THERAPY_SESSION_3_STOP,		0U,	{NULL, &format_therapy_session_3_stop}},
		{SPID_THERAPY_SESSION_4_START,		0U,	{NULL, &format_therapy_session_4_start}},
		{SPID_THERAPY_SESSION_4_STOP,		0U,	{NULL, &format_therapy_session_4_stop}},
		{SPID_THERAPY_SESSION_5_START,		0U,	{NULL, &format_therapy_session_5_start}},
		{SPID_THERAPY_SESSION_5_STOP,		0U,	{NULL, &format_therapy_session_5_stop}},
		{SPID_THERAPY_SESSION_6_START,		0U,	{NULL, &format_therapy_session_6_start}},
		{SPID_THERAPY_SESSION_6_STOP,		0U,	{NULL, &format_therapy_session_6_stop}},
};

const uint16_t parameters_list_size = (uint16_t)(sizeof(parameters_list) / sizeof(Parameter_t));

/**
 * @brief Writes/updates parameter data
 * 
 * @param virtAddress virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
 * @param p_id Parameter ID
 * @param p_data Parameter data
 * @param datalen Parameter data length
 * @return uint16_t Next EEPROM virtual address
 */
static uint16_t app_func_para_write (uint16_t virtAddress, const uint8_t* p_id, const uint8_t* p_data, uint8_t datalen) {
	uint16_t addr = virtAddress;
	Parameter_Data96bits_t data;
	uint8_t datalen_ofbuff = (uint8_t)sizeof(data.buffer);
	(void)memcpy(data.id, p_id, sizeof(data.id));

	EE_Status ee_status = EE_OK;
	for(uint8_t i = 0;i < datalen;i+=datalen_ofbuff) {
		data.buffer = 0;
		uint8_t datalen_inbuff = datalen - i;
		if (datalen_inbuff < datalen_ofbuff) {
			(void)memcpy((uint8_t*)&data.buffer, &p_data[i], datalen_inbuff);
		}
		else {
			(void)memcpy((uint8_t*)&data.buffer, &p_data[i], datalen_ofbuff);
		}

	    ee_status = EE_WriteVariable96bits(addr, &data.buffer);

	    /* Start cleanup mode, if cleanup is needed */
	    if (((uint16_t)ee_status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) {
	    	ee_status = EE_CleanUp();
	    }

	    if (ee_status != EE_OK) {
	    	Error_Handler();
	    }
	    addr++;
	}
	HAL_ERROR_CHECK(HAL_ICACHE_Invalidate());
	return addr;
}

/**
 * @brief Returns the last stored parameter data
 * 
 * @param virtAddress virtual address on 16 bits (can't be 0x0000 or 0xFFFF)
 * @param p_id Parameter ID
 * @param p_data Parameter data
 * @param datalen Parameter data length
 * @return uint16_t Next EEPROM virtual address
 */
static uint16_t app_func_para_read (uint16_t virtAddress, uint8_t* p_id, uint8_t* p_data, uint8_t datalen) {
	uint16_t addr = virtAddress;
	Parameter_Data96bits_t data = {
			.id = {0U,0U,0U,0U}
	};
	uint8_t datalen_ofbuff = (uint8_t)sizeof(data.buffer);

	EE_Status ee_status = EE_OK;
	for(uint8_t i = 0U;i < datalen;i+=datalen_ofbuff) {
	    ee_status = EE_ReadVariable96bits(addr, &data.buffer);
	    if (((uint16_t)ee_status & EE_STATUSMASK_ERROR) == EE_STATUSMASK_ERROR) {
	    	Error_Handler();
	    }

	    uint8_t datalen_inbuff = datalen - i;
	    if (datalen < datalen_ofbuff) {
	    	(void)memcpy(p_data, (uint8_t*)&data.buffer, datalen);
	    }
	    else if (datalen_inbuff < datalen_ofbuff) {
			(void)memcpy(&p_data[i], (uint8_t*)&data.buffer, datalen_inbuff);
		}
		else {
			(void)memcpy(&p_data[i], (uint8_t*)&data.buffer, datalen_ofbuff);
		}
	    addr++;
	}
	if (p_id != NULL) {
		(void)memcpy(p_id, data.id, sizeof(data.id));
	}
	return addr;
}

/**
 * @brief Get the corresponding parameter based on the parameter ID
 *
 * @param p_id Parameter ID
 * @return Parameter_t* The parameter corresponding to the parameter ID
 */
static Parameter_t* app_func_para_get(const uint8_t* p_id) {
	Parameter_t* p_para = NULL;

	if (p_id != NULL) {
		uint16_t amount = (uint16_t)(sizeof(parameters_list) / sizeof(Parameter_t));
		for(uint16_t i=0;i<amount;i++) {
			if (memcmp(parameters_list[i].id, p_id, LEN_ID) == 0) {
				p_para = &parameters_list[i];
				break;
			}
		}
	}
	return p_para;
}

/**
 * @brief Parameter buffer initialization
 * 
 */
void app_func_para_init (void) {
    uint32_t pid = 0;
    uint16_t fw_elements = 0;
    uint16_t fw_id_crc = (uint16_t)hcrc.Instance->INIT;
    uint8_t fw_id_times = 0;
    __HAL_CRC_DR_RESET(&hcrc);

	for(uint16_t i=0;i<parameters_list_size;i++) {
		uint8_t datalen = app_func_para_datalen_get(parameters_list[i].id);
		if ((datalen % BYTE_PER_ADDRESS) > 0U) {
			fw_id_times = (datalen / BYTE_PER_ADDRESS) + 1U;
		}
		else {
			fw_id_times = datalen / BYTE_PER_ADDRESS;
		}
		for(uint8_t j=0;j<fw_id_times;j++) {
			(void)memcpy((uint8_t*)&pid, parameters_list[i].id, LEN_ID);
			fw_id_crc = (uint16_t)HAL_CRC_Accumulate(&hcrc, &pid, LEN_ID);
			fw_elements++;
		}
	}

	HAL_ERROR_CHECK(HAL_FLASH_Unlock());
	EE_Status ee_status = EE_OK;
    ee_status = EE_Init(EE_FORCED_ERASE);
    if(ee_status != EE_OK) {
    	Error_Handler();
    }

	ee_status = EE_OK;
	Parameter_Data96bits_t eeData;
	uint16_t ee_elements = 0;
	uint16_t ee_id_crc = (uint16_t)hcrc.Instance->INIT;
	__HAL_CRC_DR_RESET(&hcrc);

	uint16_t virtAddr = 0;
	while(ee_status == EE_OK) {
		virtAddr++;
		ee_status = EE_ReadVariable96bits(virtAddr, &eeData.buffer);
		if (ee_status == EE_OK) {
			(void)memcpy((uint8_t*)&pid, eeData.id, LEN_ID);
			__HAL_CRC_INITIALCRCVALUE_CONFIG(&hcrc, ee_id_crc);
			ee_id_crc = (uint16_t)HAL_CRC_Accumulate(&hcrc, &pid, LEN_ID);
			__HAL_CRC_INITIALCRCVALUE_CONFIG(&hcrc, hcrc.Init.InitValue);
			ee_elements++;
		}
	}

//	fw_elements = ee_elements + 1;

	if (((virtAddr == 1U) && (ee_status == EE_NO_DATA)) || (memcmp(&fw_id_crc, &ee_id_crc, sizeof(uint16_t)) != 0) || (fw_elements != ee_elements)) {
	    ee_status = EE_Format(EE_FORCED_ERASE);
	    if(ee_status != EE_OK) {
	    	Error_Handler();
	    }
		virtAddr = 1;
		for(uint16_t i=0;i<parameters_list_size;i++) {
			parameters_list[i].virtAddress = virtAddr;
			uint8_t* p_data_def = NULL;
			uint8_t datalen = 0;
			if (parameters_list[i].format.spec_Rawdata != NULL) {
				Parameter_Format_Rawdata_t* format = parameters_list[i].format.spec_Rawdata;
				p_data_def = (uint8_t*)(format->data_def);
				datalen = format->data_len;
			}
			else {
				Parameter_Format_Value_t* format = parameters_list[i].format.spec_Value;
				p_data_def = (uint8_t*)(&format->def);
				datalen = (uint8_t)LEN_FORMAT_VALUE;
			}
			virtAddr = app_func_para_write(virtAddr, parameters_list[i].id, p_data_def, datalen);
		}
	}
	else {
		virtAddr = 1;
		for(uint16_t i=0;i<parameters_list_size;i++) {
			parameters_list[i].virtAddress = virtAddr;
			uint8_t datalen = app_func_para_datalen_get(parameters_list[i].id);
			uint16_t addroffset = (uint16_t)datalen / BYTE_PER_ADDRESS;
			if ((datalen % BYTE_PER_ADDRESS) > 0U) {
				virtAddr += (addroffset + 1U);
			}
			else {
				virtAddr += addroffset;
			}
		}

		uint8_t def_ipg_fw_ver[6] = APP_FW_VER_STR;
		uint8_t hp_ipg_fw_ver[6];
		app_func_para_data_get((const uint8_t*)HPID_IPG_FW_VERSION, hp_ipg_fw_ver, (uint8_t)sizeof(hp_ipg_fw_ver));
		if (memcmp(hp_ipg_fw_ver, def_ipg_fw_ver, sizeof(def_ipg_fw_ver)) != 0) {
			app_func_para_data_set((const uint8_t*)HPID_IPG_FW_VERSION, NULL);
		}
	}
	HAL_ERROR_CHECK(HAL_FLASH_Lock());
}

/**
 * @brief Get the data type based on the parameter ID
 *
 * @param p_id Parameter ID
 * @return uint8_t The data type corresponding to the parameter ID
 */
uint8_t app_func_para_datatype_get(const uint8_t* p_id) {
	uint8_t type = FORMAT_TYPE_INVALID;
	if (p_id != NULL) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			if (p_para->format.spec_Rawdata != NULL) {
				type = FORMAT_TYPE_RAWDATA;
			}
			else if (p_para->format.spec_Value != NULL) {
				type = FORMAT_TYPE_VALUE;
			}
			else {
				type = FORMAT_TYPE_INVALID;
			}
		}
	}
	return type;
}

/**
 * @brief Get the length of the data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @return uint8_t The data length corresponding to the parameter ID
 */
uint8_t app_func_para_datalen_get(const uint8_t* p_id) {
	uint8_t datalen = 0;
	if (p_id != NULL) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			if ((p_para->format.spec_Rawdata != NULL)) {
				Parameter_Format_Rawdata_t* format = p_para->format.spec_Rawdata;
				datalen = format->data_len;
			}
			else {
				datalen = (uint8_t)LEN_FORMAT_VALUE;
			}
		}
	}
	return datalen;
}

/**
 * @brief Set the data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @param p_data The data corresponding to the parameter ID
 */
void app_func_para_data_set(const uint8_t* p_id, uint8_t* p_data) {
	if (p_id != NULL) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			uint8_t datalen = 0;
			uint8_t* p_data_set = p_data;
			if (p_para->format.spec_Rawdata != NULL) {
				Parameter_Format_Rawdata_t* format = p_para->format.spec_Rawdata;
				datalen = format->data_len;
				if (p_data_set == NULL) {
					p_data_set = (uint8_t*)format->data_def;
				}
			}
			else {
				datalen = (uint8_t)LEN_FORMAT_VALUE;
				if (p_data_set == NULL) {
					Parameter_Format_Value_t* format = p_para->format.spec_Value;
					p_data_set = (uint8_t*)&format->def;
				}
			}
			HAL_ERROR_CHECK(HAL_FLASH_Unlock());
			(void)app_func_para_write(p_para->virtAddress, p_id, p_data_set, datalen);
			HAL_ERROR_CHECK(HAL_FLASH_Lock());
		}
	}
}

/**
 * @brief Get the data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @param p_data The data corresponding to the parameter ID
 * @param buff_size The data buffer size of p_data
 */
void app_func_para_data_get(const uint8_t* p_id, uint8_t* p_data, uint8_t buff_size) {
	if ((p_id != NULL) && (p_data != NULL)) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			uint8_t datalen = 0;
			if (p_para->format.spec_Rawdata != NULL) {
				Parameter_Format_Rawdata_t* format = p_para->format.spec_Rawdata;
				datalen = format->data_len;
			}
			else {
				datalen = (uint8_t)LEN_FORMAT_VALUE;
			}
			if (buff_size < datalen) {
				datalen = buff_size;
			}
			(void)app_func_para_read(p_para->virtAddress, NULL, p_data, datalen);
		}
	}
}

/**
 * @brief Get the default data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @param p_data The default data corresponding to the parameter ID
 */
void app_func_para_defdata_get(const uint8_t* p_id, uint8_t* p_data) {
	if ((p_id != NULL) && (p_data != NULL)) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			if (p_para->format.spec_Rawdata != NULL) {
				Parameter_Format_Rawdata_t* format = p_para->format.spec_Rawdata;
				(void)memcpy(p_data, format->data_def, format->data_len);
			}
			else {
				Parameter_Format_Value_t* format = p_para->format.spec_Value;
				(void)memcpy(p_data, (uint8_t*)&format->def, LEN_FORMAT_VALUE);
			}
		}
	}
}

/**
 * @brief Get the step size of the parameter (The parameter data format is value)
 *
 * @param p_id Parameter ID
 * @return _Float64 The step size obtained from this parameter
 */
_Float64 app_func_para_stepsize_get(const uint8_t* p_id) {
	_Float64 stepsize = 0.0;
	if (p_id != NULL) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			if (p_para->format.spec_Value != NULL) {
				Parameter_Format_Value_t* format = p_para->format.spec_Value;
				stepsize = format->step_size;
			}
		}
	}
	return stepsize;
}

/**
 * @brief Check whether the parameter value is within the range (The parameter data format is value)
 *
 * @param p_id Parameter ID
 * @param val Value to be confirmed
 * @return true The value is within the range
 * @return false The value is not within the range
 */
bool app_func_para_val_in_range(const uint8_t* p_id, _Float64 val) {
	bool in_range = false;
	if (p_id != NULL) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			if (p_para->format.spec_Value != NULL) {
				Parameter_Format_Value_t* format = p_para->format.spec_Value;
				if ((val <= format->max) && (val >= format->min)) {
					in_range = true;
				}
			}
		}
	}
	return in_range;
}

/**
 * @brief Quantizes a parameter value to the nearest multiple of a step and clips it within a specified range.
 *
 * @param p_id Parameter ID
 * @param val The original value to be quantized and clipped.
 * @return _Float64 The quantized and clipped value as a double.
 */
_Float64 app_func_para_val_quant_clip(const uint8_t* p_id, _Float64 val) {
	if (p_id != NULL) {
		Parameter_t* p_para = app_func_para_get(p_id);
		if (p_para != NULL) {
			if (p_para->format.spec_Value != NULL) {
				Parameter_Format_Value_t* format = p_para->format.spec_Value;
				_Float64 quantizedValue = round(val / format->step_size) * format->step_size;

				if (quantizedValue < format->min) {
					return format->min;
				} else if (quantizedValue > format->max) {
					return format->max;
				} else {
					return quantizedValue;
				}
			}
		}
	}
	return 0;
}
