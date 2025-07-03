/**
 * @file app_mode_ble_connection.c
 * @brief This file provides management of the BLE connection mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_ble_connection.h"
#include "app_config.h"

#define SAMPLE_POINTS		100U
#define SAMPLE_FREQ_ECG		256U
#define SAMPLE_FREQ_EMG		5000U

#define BLE_ACCESS_TIME_MS	1000

int32_t idle_connection_ms_timer = -1;
int32_t disconnect_request_ms_timer = -1;

static int32_t ble_access_ms_timer = -1;

static _Float64 ble_idle_connection_f = 0.0;

static bool sw_reset = false;

static bool stim_en = false;
static bool sens_en = false;

static uint8_t sensor_resp_payload[LEN_RESP_PAYLOAD_MAX];
static Cmd_Resp_t sensor_resp;

/**
 * @brief Parser for request commands in BLE connection mode, used to communicate with the remote end
 * 
 * @param req Request command to be parsed
 * @return Cmd_Resp_t The response command to be replied after parsing the request command
 */
static Cmd_Resp_t app_mode_ble_conn_cmd_parser(Cmd_Req_t req) {
	Cmd_Resp_t resp = {
			.Opcode 		= req.Opcode,
			.Status 		= STATUS_SUCCESS,
			.Payload 		= NULL,
			.PayloadLen 	= 0,
	};
	uint8_t len_payload_min = 0;
	uint8_t len_payload_max = 0;
	uint8_t	user_class_cmd = 0;

	uint8_t user_class = app_mode_ble_act_userclass_get();

	app_func_para_data_get((const uint8_t*)HPID_BLE_IDLE_CONNECTION, (uint8_t*)&ble_idle_connection_f, (uint8_t)sizeof(ble_idle_connection_f));
	ble_idle_connection_f *= 1000.0;
	idle_connection_ms_timer = (int32_t)ble_idle_connection_f;

	static RTC_TimeTypeDef rtc_time;
	static RTC_DateTypeDef rtc_date;
	static uint8_t resp_payload[LEN_RESP_PAYLOAD_MAX];

	switch(req.Opcode) {
	case OP_SET_START_STATE:
	{
		len_payload_min = 2;
		len_payload_max = 2;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			uint16_t state = STATE_INVALID;
			(void)memcpy((void*)&state, (void*)req.Payload, sizeof(uint16_t));
			switch(state) {
			case STATE_SLEEP:
			case STATE_ACT:
			case STATE_ACT_MODE_BLE_ACT:
			case STATE_ACT_MODE_THERAPY_SESSION:
			case STATE_ACT_MODE_IMPED_TEST:
			case STATE_ACT_MODE_BATT_TEST:
			case STATE_ACT_MODE_DVT:
			{
				Sys_Config_t sc = {
						.DefaultState = DEFAULT_STATE,
						.StartState = state,
				};
				bsp_fram_write(ADDR_SYS_CONFIG, (uint8_t*)&sc, sizeof(sc), true);
				sw_reset = true;
			}	break;

			default:
			{
				resp.Status = STATUS_INVALID;
			}
				break;
			}
		}
	}
		break;

	case OP_READ_BLE_ADVANCE:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			BLE_Advance_t* p_advance = (BLE_Advance_t*)resp_payload;
			app_func_para_data_get((const uint8_t*)BPID_BLE_PASSKEY, p_advance->passkey, (uint8_t)sizeof(p_advance->passkey));
			app_func_para_data_get((const uint8_t*)BPID_BLE_WHITELIST, &p_advance->whitelist_enable, (uint8_t)sizeof(p_advance->whitelist_enable));

			resp.PayloadLen = (uint8_t)sizeof(BLE_Advance_t);
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_WRITE_BLE_ADVANCE:
	{
		len_payload_min = (uint8_t)sizeof(BLE_Advance_t);
		len_payload_max = (uint8_t)sizeof(BLE_Advance_t);
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			BLE_Advance_t advance;
			(void)memcpy((void*)&advance, (void*)req.Payload, sizeof(BLE_Advance_t));
			app_func_para_data_set((const uint8_t*)BPID_BLE_PASSKEY, advance.passkey);
			app_func_para_data_set((const uint8_t*)BPID_BLE_WHITELIST, &advance.whitelist_enable);
		}
	}
		break;

	case OP_AUTH_FW_IMAGE:
	{
		len_payload_min = (uint8_t)sizeof(ECDSA_Data_t);
		len_payload_max = (uint8_t)sizeof(ECDSA_Data_t);
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			ECDSA_Data_t data;
			(void)memcpy((void*)&data, (void*)req.Payload, sizeof(ECDSA_Data_t));
			if (app_func_auth_verify_sign_admin(data) == false) {
				resp.Status = STATUS_INVALID;
			}
			else {
				app_func_sm_current_state_set(STATE_ACT_MODE_OAD);
			}
		}
	}
		break;

	case OP_SHUTDOWN_SYSTEM:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			app_func_sm_current_state_set(STATE_SHUTDOWN);
		}
	}
		break;

	case OP_REBOOT_SYSTEM:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			app_func_sm_current_state_set(STATE_ACT_MODE_BSL);
		}
	}
		break;

	case OP_BLE_DISCONNECT_REQUEST:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			_Float64 ble_disconnect_request_f = 0.0;
			app_func_para_data_get((const uint8_t*)HPID_BLE_DISCONNECT_REQUEST, (uint8_t*)&ble_disconnect_request_f, (uint8_t)sizeof(ble_disconnect_request_f));
			ble_disconnect_request_f *= 1000.0;
			disconnect_request_ms_timer = (int32_t)ble_disconnect_request_f;
		}
	}
		break;

	case OP_START_SCHED_THERAPY_SESSION:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_PATIENT;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			app_func_sm_schd_therapy_enable(true);
		}
	}
		break;

	case OP_END_SCHED_THERAPY_SESSION:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_PATIENT;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			app_func_sm_schd_therapy_enable(false);
		}
	}
		break;

	case OP_START_MANUAL_THERAPY_SESSION:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			if (app_mode_therapy_start() != true) {
				resp.Status = STATUS_INVALID;
			}
			else {
				stim_en = true;
			}
		}
	}
		break;

	case OP_STOP_MANUAL_THERAPY_SESSION:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			app_mode_therapy_stop();
			stim_en = false;
		}
	}
		break;

	case OP_MEASURE_IMPEDANCE:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else if (sens_en) {
			resp.Status = STATUS_INVALID;
		}
		else {
			uint16_t* p_imp = (uint16_t*)resp_payload;
			*p_imp = (uint16_t)app_mode_impedance_test_get();

			resp.PayloadLen = (uint8_t)sizeof(uint16_t);
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_MEASURE_BATTERY_VOLTAGE:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else if (sens_en) {
			resp.Status = STATUS_INVALID;
		}
		else {
			uint16_t* p_vbatA = (uint16_t*)resp_payload;
			uint16_t* p_vbatB = &p_vbatA[1];
			app_mode_battery_test_volt_get(p_vbatA, p_vbatB);

			resp.PayloadLen = (uint8_t)(sizeof(uint16_t) + sizeof(uint16_t));
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_MEASURE_SENSOR_VOLTAGE:
	{
		len_payload_min = 1;
		len_payload_max = 5;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen != len_payload_min) && (req.PayloadLen != len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			uint8_t sensorID = req.Payload[0];
			float specifySamplingFrequency = 0.0;
			if (req.PayloadLen == len_payload_max) {
				memcpy(&specifySamplingFrequency, &req.Payload[1], sizeof(float));
				if (specifySamplingFrequency < 1.5f || specifySamplingFrequency > 6553.5f) {
					resp.Status = STATUS_INVALID;
					break;
				}
			}

			uint16_t samplingFrequency_hz;
			uint8_t* buff = &sensor_resp_payload[1];
			sensor_resp_payload[0] = 0U;
			uint8_t bufferSize = SAMPLE_POINTS * sizeof(uint16_t);

			if (sensorID == SENSOR_ID_EMG2) {
			    _Float64 cathode_electrode_number = 0.0;
			    _Float64 anode_electrode_number = 0.0;
			    app_func_para_data_get((const uint8_t*)SPID_CATHODE_ELECTRODE_NUMBER, (uint8_t*)&cathode_electrode_number, sizeof(cathode_electrode_number));
			    app_func_para_data_get((const uint8_t*)SPID_ANODE_ELECTRODE_NUMBER, (uint8_t*)&anode_electrode_number, sizeof(anode_electrode_number));

			    bool occupied[10] = { false };
			    occupied[(uint8_t)cathode_electrode_number] = true;
			    occupied[(uint8_t)anode_electrode_number] = true;

			    bool groupA_available = !(occupied[6] || occupied[8]);
			    bool groupB_available = !(occupied[5] || occupied[7]);

			    if (!groupA_available && !groupB_available) {
			        resp.Status = STATUS_INVALID;
			    }
			    else {
			        uint8_t mux_ch = groupA_available ? SENSOR_ID_EMG2_CH68 : SENSOR_ID_EMG2_CH57;

			        sens_en = true;
			        if (specifySamplingFrequency == 0.0) {
			        	samplingFrequency_hz = SAMPLE_FREQ_EMG;
			        }
			        else {
			        	samplingFrequency_hz = (uint16_t)(specifySamplingFrequency);
			        }
			        app_func_meas_sensor_enable(sensorID, true);
			        app_func_meas_sensor_mux_enable(mux_ch);
			        app_func_meas_sensor_sampling(sensorID, buff, bufferSize, samplingFrequency_hz);

			        resp.PayloadLen = bufferSize + 1U;
			        resp.Payload = sensor_resp_payload;
			        sensor_resp = resp;
			    }
			}
			else if (sensorID == SENSOR_ID_ECG_HR || sensorID == SENSOR_ID_ECG_RR || sensorID == SENSOR_ID_EMG1) {
			    sens_en = true;
		        if (specifySamplingFrequency == 0.0) {
				    samplingFrequency_hz = (sensorID == SENSOR_ID_EMG1) ? SAMPLE_FREQ_EMG : SAMPLE_FREQ_ECG;
		        }
		        else {
		        	samplingFrequency_hz = (uint16_t)specifySamplingFrequency;
		        }
			    app_func_meas_sensor_enable(sensorID, true);
			    app_func_meas_sensor_mux_enable(sensorID);
			    app_func_meas_sensor_sampling(sensorID, buff, bufferSize, samplingFrequency_hz);

			    resp.PayloadLen = bufferSize + 1U;
			    resp.Payload = sensor_resp_payload;
			    sensor_resp = resp;
			}
			else if (sensorID == SENSOR_ID_IDLE) {
			    sens_en = false;
			    app_func_meas_sensor_enable(SENSOR_ID_ECG_HR, false);
			    app_func_meas_sensor_enable(SENSOR_ID_ECG_RR, false);
			    app_func_meas_sensor_enable(SENSOR_ID_EMG1, false);
			    app_func_meas_sensor_enable(SENSOR_ID_EMG2, false);
			    bsp_mux_enable(false);
			}
			else {
			    resp.Status = STATUS_INVALID;
			}
		}
	}
		break;

	case OP_READ_HARDWARE_PARAMETERS:
	case OP_READ_STIMULATION_PARAMETERS:
	{
		len_payload_min = LEN_ID;
		len_payload_max = LEN_ID;
		user_class_cmd = USER_CLASS_ADMIN;

		if (req.Opcode == OP_READ_STIMULATION_PARAMETERS) {
			user_class_cmd = USER_CLASS_CLINICIAN;
		}

		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			uint8_t* parameter_id = req.Payload;
			(void)memcpy(resp_payload, parameter_id, LEN_ID);

			if (req.Opcode == OP_READ_HARDWARE_PARAMETERS &&
					memcmp(parameter_id, (uint8_t*)HPID_PREFIX, 2) != 0) {
				resp.Status = STATUS_INVALID;
			}
			else if (req.Opcode == OP_READ_STIMULATION_PARAMETERS &&
					memcmp(parameter_id, (uint8_t*)SPID_PREFIX, 2) != 0 &&
					memcmp(parameter_id, (uint8_t*)SPID_PREFIX_ST, 2) != 0) {
				resp.Status = STATUS_INVALID;
			}
			else {
				uint8_t datatype = app_func_para_datatype_get((const uint8_t*)parameter_id);
				if (datatype == FORMAT_TYPE_RAWDATA) {
					app_func_para_data_get((const uint8_t*)parameter_id, &resp_payload[LEN_ID], (uint8_t)(LEN_RESP_PAYLOAD_MAX - LEN_ID));
					resp.PayloadLen = LEN_ID + app_func_para_datalen_get((const uint8_t*)parameter_id);;
					resp.Payload = resp_payload;
				}
				else if (datatype == FORMAT_TYPE_VALUE) {
					_Float64 val = 0.0;
					app_func_para_data_get((const uint8_t*)parameter_id, (uint8_t*)&val, (uint8_t)sizeof(val));
					_Float64 stepsize = app_func_para_stepsize_get((const uint8_t*)parameter_id);
					_Float64 step_f = val / stepsize;
					uint16_t step = (uint16_t)step_f;
					(void)memcpy((void*)&resp_payload[LEN_ID], (void*)&step, LEN_STEP);
					resp.PayloadLen = LEN_ID + (uint8_t)LEN_STEP;
					resp.Payload = resp_payload;
				}
				else {
					resp.Status = STATUS_INVALID;
				}
			}
		}
	}
		break;

	case OP_WRITE_HARDWARE_PARAMETERS:
	case OP_WRITE_STIMULATION_PARAMETERS:
	{
		len_payload_min = LEN_ID;
		len_payload_max = LEN_REQ_PAYLOAD_MAX;
		user_class_cmd = USER_CLASS_ADMIN;

		if (req.Opcode == OP_WRITE_STIMULATION_PARAMETERS) {
			user_class_cmd = USER_CLASS_CLINICIAN;
		}

		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			uint8_t* parameter_id = req.Payload;
			uint8_t* p_data = NULL;

			if (req.Opcode == OP_WRITE_HARDWARE_PARAMETERS &&
					memcmp(parameter_id, (uint8_t*)HPID_PREFIX, 2) != 0) {
				resp.Status = STATUS_INVALID;
			}
			else if (req.Opcode == OP_WRITE_STIMULATION_PARAMETERS &&
					memcmp(parameter_id, (uint8_t*)SPID_PREFIX, 2) != 0 &&
					memcmp(parameter_id, (uint8_t*)SPID_PREFIX_ST, 2) != 0) {
				resp.Status = STATUS_INVALID;
			}
			else {
				uint8_t datatype = app_func_para_datatype_get((const uint8_t*)parameter_id);
				if (req.PayloadLen == LEN_ID) {
					app_func_para_data_set((const uint8_t*)parameter_id, NULL);
				}
				else if (datatype == FORMAT_TYPE_RAWDATA) {
					uint8_t datalen = app_func_para_datalen_get((const uint8_t*)parameter_id);
					if (req.PayloadLen != (LEN_ID + datalen)) {
						resp.Status = STATUS_PAYLOAD_LEN_ERR;
					}
					else {
						p_data = &req.Payload[LEN_ID];
						app_func_para_data_set((const uint8_t*)parameter_id, p_data);
						app_func_logs_parameter_write(parameter_id, FORMAT_TYPE_RAWDATA, p_data, datalen);
					}
				}
				else if (datatype == FORMAT_TYPE_VALUE) {
					if (req.PayloadLen != (LEN_ID + LEN_STEP)) {
						resp.Status = STATUS_PAYLOAD_LEN_ERR;
					}
					else {
						p_data = &req.Payload[LEN_ID];
						uint16_t steps = 0U;
						(void)memcpy((void*)&steps, (void*)p_data, sizeof(uint16_t));
						_Float64 steps_f = (_Float64)steps;
						_Float64 stepsize = app_func_para_stepsize_get((const uint8_t*)parameter_id);
						_Float64 val = steps_f * stepsize;
						if (app_func_para_val_in_range((const uint8_t*)parameter_id, val) == false) {
							resp.Status = STATUS_INVALID;
						}
						else {
							app_func_para_data_set((const uint8_t*)parameter_id, (uint8_t*)&val);
							app_func_logs_parameter_write(parameter_id, FORMAT_TYPE_VALUE, (uint8_t*)&val, (uint16_t)LEN_FORMAT_VALUE);
						}
					}
				}
				else {
					resp.Status = STATUS_INVALID;
				}
			}
		}
	}
		break;

	case OP_READ_IPG_LOG:
	{
		len_payload_min = 7;
		len_payload_max = 7;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			resp.PayloadLen = app_func_logs_read(req.Payload, resp_payload);
			resp.Payload = resp_payload;
		}
	}
		break;

	case OP_ERASE_IPG_LOG:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			app_func_logs_erase();
		}
	}
		break;

	case OP_READ_TIME_AND_DATE:
	{
		len_payload_min = 0;
		len_payload_max = 0;
		user_class_cmd = USER_CLASS_ADMIN;
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			HAL_ERROR_CHECK(HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN));
			HAL_ERROR_CHECK(HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN));
			uint8_t date_time[6] = {rtc_date.Year, rtc_date.Month, rtc_date.Date, rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds};
			resp.PayloadLen = (uint8_t)sizeof(date_time);
			resp.Payload = (uint8_t*)date_time;
		}
	}
		break;

	case OP_WRITE_TIME_AND_DATE:
	{
		len_payload_min = 6;
		len_payload_max = 6;
		user_class_cmd = USER_CLASS_ADMIN;
	if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else if (user_class < user_class_cmd) {
			resp.Status = STATUS_USER_CLASS_ERR;
		}
		else {
			if(!IS_RTC_YEAR(*(req.Payload)))
				resp.Status = STATUS_INVALID;

			if(!IS_RTC_MONTH(*(req.Payload + 1)))
				resp.Status = STATUS_INVALID;

			if(!IS_RTC_DATE(*(req.Payload + 2)))
				resp.Status = STATUS_INVALID;

			if(!IS_RTC_HOUR24(*(req.Payload + 3)))
				resp.Status = STATUS_INVALID;

			if(!IS_RTC_MINUTES(*(req.Payload + 4)))
				resp.Status = STATUS_INVALID;

			if(!IS_RTC_SECONDS(*(req.Payload + 5)))
				resp.Status = STATUS_INVALID;

			if (resp.Status == STATUS_SUCCESS) {
				rtc_date.Year 		= req.Payload[0];
				rtc_date.Month 		= req.Payload[1];
				rtc_date.Date 		= req.Payload[2];
				rtc_time.Hours 		= req.Payload[3];
				rtc_time.Minutes 	= req.Payload[4];
				rtc_time.Seconds 	= req.Payload[5];
				HAL_ERROR_CHECK(HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN));
				HAL_ERROR_CHECK(HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN));
			}
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
 * @brief Handler for BLE connection mode
 * 
 */
void app_mode_ble_conn_handler(void) {
	uint16_t curr_state = app_func_sm_current_state_get();
	app_func_command_req_parser_set(&app_mode_ble_conn_cmd_parser);
	uint8_t curr_ble_state = app_func_ble_curr_state_get();

	app_func_para_data_get((const uint8_t*)HPID_BLE_IDLE_CONNECTION, (uint8_t*)&ble_idle_connection_f, (uint8_t)sizeof(ble_idle_connection_f));
	ble_idle_connection_f *= 1000.0;
	idle_connection_ms_timer = (int32_t)ble_idle_connection_f;
	disconnect_request_ms_timer = -1;
	ble_access_ms_timer = BLE_ACCESS_TIME_MS;

	while(curr_state == STATE_ACT_MODE_BLE_CONN) {
		bsp_wdg_refresh();
		if (((idle_connection_ms_timer == 0) || (disconnect_request_ms_timer == 0)) && (app_mode_therapy_confirm() == false)) {
			app_func_ble_disconnect();
			idle_connection_ms_timer = -1;
			disconnect_request_ms_timer = -1;
			app_func_sm_current_state_set(STATE_ACT);
		}
		else if (sens_en == true && bsp_adc_sampling_is_completed() == true) {
			app_func_meas_sensor_continue();
			sensor_resp_payload[0] = (sensor_resp_payload[0] + 1U) % 100U;
			app_func_command_resp_send(sensor_resp);
			idle_connection_ms_timer = (int32_t)ble_idle_connection_f;
			bsp_sp_cmd_handler();
		}
		else if (ble_access_ms_timer == 0) {
			app_func_ble_new_state_get();
			while(!bsp_sp_cmd_handler()) {
				HAL_Delay(1);
			}
			ble_access_ms_timer = BLE_ACCESS_TIME_MS;
		}

		if (sw_reset) {
			HAL_NVIC_SystemReset();
		}
		curr_ble_state = app_func_ble_curr_state_get();
		if (curr_state != STATE_ACT_MODE_BLE_CONN) {
			bsp_wdg_refresh();
			HAL_Delay(100);
		}
		else if (curr_ble_state == BLE_STATE_ADV_STOP) {
			sens_en = false;
			app_func_sm_current_state_set(STATE_ACT_MODE_BLE_ACT);
		}
		curr_state = app_func_sm_current_state_get();
	}
}

/**
 * @brief Callback for the timers in BLE connection mode, unit: ms
 * 
 */
void app_mode_ble_conn_timer_cb(void) {
	if (idle_connection_ms_timer > 0) {
		idle_connection_ms_timer--;
	}

	if (disconnect_request_ms_timer > 0) {
		disconnect_request_ms_timer--;
	}

	if (ble_access_ms_timer > 0) {
		ble_access_ms_timer--;
	}
}
