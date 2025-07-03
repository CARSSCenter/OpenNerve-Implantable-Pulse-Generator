/**
 * @file app_func_parameter.h
 * @brief This file contains all the function prototypes for the app_func_parameter.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_PARAMETER_H_
#define FUNCTIONS_INC_APP_FUNC_PARAMETER_H_
#include <stdint.h>
#include <stdbool.h>

#define	TPID_SAMPLE_ID					"TP01"		/*!< Parameter ID of "SAMPLE_ID" */

#define	BPID_BLE_PASSKEY				"BP01"		/*!< Parameter ID of "BLE_PASSKEY" */
#define	BPID_BLE_WHITELIST				"BP02"		/*!< Parameter ID of "BLE_WHITELIST" */
#define	BPID_BLE_COMPANY_ID				"BP03"		/*!< Parameter ID of "BLE_COMPANY_ID" */

#define	HPID_IPG_SERIAL_NUMBER			"HP01"		/*!< Parameter ID of "IPG_SERIAL_NUMBER" */
#define	HPID_IPG_MODEL					"HP02"		/*!< Parameter ID of "IPG_MODEL" */
#define	HPID_IPG_BLE_ID					"HP03"		/*!< Parameter ID of "IPG_BLE_ID" */
#define	HPID_IPG_PRODUCTION_LOCATION	"HP04"		/*!< Parameter ID of "IPG_PRODUCTION_LOCATION" */
#define	HPID_IPG_FW_VERSION				"HP05"		/*!< Parameter ID of "IPG_FW_VERSION" */
#define	HPID_IPG_MANUFACTURING_DATE		"HP06"		/*!< Parameter ID of "IPG_MANUFACTURING_DATE" */
#define	HPID_IPG_IMPLANTATION_DATE		"HP07"		/*!< Parameter ID of "IPG_IMPLANTATION_DATE" */
#define	HPID_LINKED_CRC_BLE_ID			"HP08"		/*!< Parameter ID of "LINKED_CRC_BLE_ID" */
#define	HPID_LINKED_CRC_KEY				"HP09"		/*!< Parameter ID of "LINKED_CRC_KEY" */
#define	HPID_LINKED_CRC_FW_VERSION		"HP10"		/*!< Parameter ID of "LINKED_CRC_FW_VERSION" */
#define	HPID_LINKED_PRC_BLE_ID			"HP11"		/*!< Parameter ID of "LINKED_PRC_BLE_ID" */
#define	HPID_LINKED_PRC_KEY				"HP12"		/*!< Parameter ID of "LINKED_PRC_KEY" */
#define	HPID_LINKED_PRC_FW_VERSION		"HP13"		/*!< Parameter ID of "LINKED_PRC_FW_VERSION" */
#define	HPID_BLE_BROADCAST_TIMEOUT		"HP14"		/*!< Parameter ID of "BLE_BROADCAST_TIMEOUT" */
#define	HPID_BLE_IDLE_CONNECTION		"HP15"		/*!< Parameter ID of "BLE_IDLE_CONNECTION" */
#define	HPID_BLE_DISCONNECT_REQUEST		"HP16"		/*!< Parameter ID of "BLE_DISCONNECT_REQUEST" */
#define	HPID_BLE_INTERVAL				"HP17"		/*!< Parameter ID of "BLE_INTERVAL" */
#define	HPID_IMPEDANCE_TEST_INTERVAL	"HP18"		/*!< Parameter ID of "IMPEDANCE_TEST_INTERVAL" */
#define	HPID_BATTERY_TEST_INTERVAL		"HP19"		/*!< Parameter ID of "BATTERY_TEST_INTERVAL" */
#define	HPID_MAGNET_SHUTDOWN_MIN_TIME	"HP20"		/*!< Parameter ID of "MAGNET_SHUTDOWN_MIN_TIME" */
#define	HPID_MAGNET_SHUTDOWN_MAX_TIME	"HP21"		/*!< Parameter ID of "MAGNET_SHUTDOWN_MAX_TIME" */
#define	HPID_MAGNET_WAKEUP_MIN_TIME		"HP22"		/*!< Parameter ID of "MAGNET_WAKEUP_MIN_TIME" */
#define	HPID_MAGNET_WAKEUP_MAX_TIME		"HP23"		/*!< Parameter ID of "MAGNET_WAKEUP_MAX_TIME" */
#define	HPID_MAGNET_RESET_MIN_TIME		"HP24"		/*!< Parameter ID of "MAGNET_RESET_MIN_TIME" */
#define	HPID_MAGNET_RESET_MAX_TIME		"HP25"		/*!< Parameter ID of "MAGNET_RESET_MAX_TIME" */
#define	HPID_BATTERY_ER_LEVEL			"HP26"		/*!< Parameter ID of "BATTERY_ER_LEVEL" */
#define	HPID_BATTERY_EOS_LEVEL			"HP27"		/*!< Parameter ID of "BATTERY_EOS_LEVEL" */
#define	HPID_LANGUAGE					"HP28"		/*!< Parameter ID of "LANGUAGE" */
#define	HPID_IDLE_DURATION				"HP29"		/*!< Parameter ID of "IDLE_DURATION" */
#define	HPID_RTC_INTERRUPT_PERIOD		"HP30"		/*!< Parameter ID of "RTC_INTERRUPT_PERIOD" */

#define	SPID_NUMBER_OF_THERAPY_SESSIONS	"SP01"		/*!< Parameter ID of "NUMBER_OF_THERAPY_SESSIONS" */
#define	SPID_PULSE_AMPLITUDE			"SP02"		/*!< Parameter ID of "PULSE_AMPLITUDE" */
#define	SPID_PULSE_WIDTH				"SP03"		/*!< Parameter ID of "PULSE_WIDTH" */
#define	SPID_PULSE_FREQUENCY			"SP04"		/*!< Parameter ID of "PULSE_FREQUENCY" */
#define	SPID_RAMP_UP_DURATION			"SP05"		/*!< Parameter ID of "RAMP_UP_DURATION" */
#define	SPID_RAMP_DOWN_DURATION			"SP06"		/*!< Parameter ID of "RAMP_DOWN_DURATION" */
#define	SPID_TRAIN_ON_DURATION			"SP07"		/*!< Parameter ID of "TRAIN_ON_DURATION" */
#define	SPID_TRAIN_OFF_DURATION			"SP08"		/*!< Parameter ID of "TRAIN_OFF_DURATION" */
#define	SPID_CATHODE_ELECTRODE_NUMBER	"SP09"		/*!< Parameter ID of "CATHODE_ELECTRODE_NUMBER" */
#define	SPID_ANODE_ELECTRODE_NUMBER		"SP10"		/*!< Parameter ID of "ANODE_ELECTRODE_NUMBER" */
#define	SPID_MAX_SAFE_AMPLITUDE			"SP11"		/*!< Parameter ID of "MAX_SAFE_AMPLITUDE" */
#define	SPID_MIN_SAFE_IMPEDANCE			"SP12"		/*!< Parameter ID of "MIN_SAFE_IMPEDANCE" */

#define	SPID_THERAPY_SESSION_1_START	"ST11"		/*!< Parameter ID of "THERAPY_SESSION_1_START" */
#define	SPID_THERAPY_SESSION_1_STOP		"ST12"		/*!< Parameter ID of "THERAPY_SESSION_1_STOP" */
#define	SPID_THERAPY_SESSION_2_START	"ST21"		/*!< Parameter ID of "THERAPY_SESSION_2_START" */
#define	SPID_THERAPY_SESSION_2_STOP		"ST22"		/*!< Parameter ID of "THERAPY_SESSION_2_STOP" */
#define	SPID_THERAPY_SESSION_3_START	"ST31"		/*!< Parameter ID of "THERAPY_SESSION_3_START" */
#define	SPID_THERAPY_SESSION_3_STOP		"ST32"		/*!< Parameter ID of "THERAPY_SESSION_3_STOP" */
#define	SPID_THERAPY_SESSION_4_START	"ST41"		/*!< Parameter ID of "THERAPY_SESSION_4_START" */
#define	SPID_THERAPY_SESSION_4_STOP		"ST42"		/*!< Parameter ID of "THERAPY_SESSION_4_STOP" */
#define	SPID_THERAPY_SESSION_5_START	"ST51"		/*!< Parameter ID of "THERAPY_SESSION_5_START" */
#define	SPID_THERAPY_SESSION_5_STOP		"ST52"		/*!< Parameter ID of "THERAPY_SESSION_5_STOP" */
#define	SPID_THERAPY_SESSION_6_START	"ST61"		/*!< Parameter ID of "THERAPY_SESSION_6_START" */
#define	SPID_THERAPY_SESSION_6_STOP		"ST62"		/*!< Parameter ID of "THERAPY_SESSION_6_STOP" */

#define	HPID_PREFIX						"HP"		/*!< The prefix of the HPID */
#define	SPID_PREFIX						"SP"		/*!< The prefix of the SPID */
#define	SPID_PREFIX_ST					"ST"		/*!< The prefix of the SPID (THERAPY_SESSION) */

#define	FORMAT_TYPE_INVALID				0U			/*!< The parameter format type is invalid */
#define	FORMAT_TYPE_RAWDATA				1U			/*!< The parameter format type is raw data */
#define	FORMAT_TYPE_VALUE				2U			/*!< The parameter format type is value */

#define LEN_ID							4U					/*!< The length of the parameter ID */
#define LEN_FORMAT_VALUE				sizeof(_Float64)	/*!< The length of the parameter value */
#define LEN_STEP						sizeof(uint16_t)	/*!< The length of the parameter step */

typedef struct {
	uint64_t buffer;					/*!< Data buffer for each virtual address */
	uint8_t id[4];						/*!< Parameter ID for each virtual address */
} Parameter_Data96bits_t;

typedef struct {
	uint8_t* 	data_def;				/*!< The default data of the parameter */
	uint8_t 	data_len;				/*!< The data length of the parameter */
} Parameter_Format_Rawdata_t;

typedef struct {
	_Float64 	min;					/*!< The minimum value of the parameter */
	_Float64 	max;					/*!< The maximum value of the parameter */
	_Float64 	def;					/*!< The default value of the parameter */
	_Float64 	step_size;				/*!< The step size of the parameter */
} Parameter_Format_Value_t;

typedef struct {
	Parameter_Format_Rawdata_t* 	spec_Rawdata;			/*!< The data specifications of the parameter(raw data) */
	Parameter_Format_Value_t* 		spec_Value;				/*!< The data specifications of the parameter(value) */
} Parameter_Format_t;

typedef struct {
	uint8_t 			id[LEN_ID];		/*!< The ID of the parameter */
	uint16_t 			virtAddress;	/*!< The virtual address of the parameter */
	Parameter_Format_t 	format;			/*!< The data format of the parameter */
} Parameter_t;

/**
 * @brief Parameter buffer initialization
 * 
 */
void app_func_para_init (void);

/**
 * @brief Get the data type based on the parameter ID
 *
 * @param p_id Parameter ID
 * @return uint8_t The data type corresponding to the parameter ID
 */
uint8_t app_func_para_datatype_get(const uint8_t* p_id);
/**
 * @brief Get the length of the data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @return uint8_t The data length corresponding to the parameter ID
 */
uint8_t app_func_para_datalen_get(const uint8_t* p_id);
/**
 * @brief Set the data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @param p_data The data corresponding to the parameter ID
 */
void app_func_para_data_set(const uint8_t* p_id, uint8_t* p_data);

/**
 * @brief Get the data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @param p_data The data corresponding to the parameter ID
 * @param buff_size The data buffer size of p_data
 */
void app_func_para_data_get(const uint8_t* p_id, uint8_t* p_data, uint8_t buff_size);

/**
 * @brief Get the default data based on the parameter ID
 *
 * @param p_id Parameter ID
 * @param p_data The default data corresponding to the parameter ID
 */
void app_func_para_defdata_get(const uint8_t* p_id, uint8_t* p_data);

/**
 * @brief Get the step size of the parameter (The parameter data format is value)
 *
 * @param p_id Parameter ID
 * @return _Float64 The step size obtained from this parameter
 */
_Float64 app_func_para_stepsize_get(const uint8_t* p_id);

/**
 * @brief Check whether the parameter value is within the range (The parameter data format is value)
 *
 * @param p_id Parameter ID
 * @param val Value to be confirmed
 * @return true The value is within the range
 * @return false The value is not within the range
 */
bool app_func_para_val_in_range(const uint8_t* p_id, _Float64 val);

/**
 * @brief Quantizes a parameter value to the nearest multiple of a step and clips it within a specified range.
 *
 * @param p_id Parameter ID
 * @param val The original value to be quantized and clipped.
 * @return _Float64 The quantized and clipped value as a double.
 */
_Float64 app_func_para_val_quant_clip(const uint8_t* p_id, _Float64 val);

#endif /* FUNCTIONS_INC_APP_FUNC_PARAMETER_H_ */
