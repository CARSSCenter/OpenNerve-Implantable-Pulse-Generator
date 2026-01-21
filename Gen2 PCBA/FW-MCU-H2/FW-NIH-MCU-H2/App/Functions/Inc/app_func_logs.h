/**
 * @file app_func_logs.h
 * @brief This file contains all the function prototypes for the app_func_logs.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_LOGS_H_
#define FUNCTIONS_INC_APP_FUNC_LOGS_H_
#include <stdint.h>
#include <stdbool.h>

#define EVENT_SHORT_CIRCUIT				"SC   "		/*!< Label message for event "SHORT_CIRCUIT" */
#define EVENT_OPEN_CIRCUIT				"OC   "		/*!< Label message for event "OPEN_CIRCUIT" */
#define EVENT_ER						"ER   "		/*!< Label message for event "ER" */
#define EVENT_EOS						"EOS  "		/*!< Label message for event "EOS" */
#define EVENT_MAGNET_DETECTION			"MD   "		/*!< Label message for event "MAGNET_DETECTION" */
#define EVENT_UNRESPONSIVE_FUNCTION		"UF   "		/*!< Label message for event "UNRESPONSIVE_FUNCTION" */
#define EVENT_LOWER_STIM_AMP			"LSA  "		/*!< Label message for event "LOWER_STIM_AMP" */
#define EVENT_HIGH_IMPED				"HI   "		/*!< Label message for event "HIGH_IMPED" */
#define EVENT_NORMAL_IMPED				"NI   "		/*!< Label message for event "NORMAL_IMPED" */

#define LEN_DATA_TYPE_STR				4U			/*!< The length of Label message for data type */
#define LEN_EVENT_TYPE_STR				5U			/*!< The length of Label message for event type */

typedef void (*Log_Event_Write_Callback)(void);

typedef struct {
	uint32_t 	LogPointer;							/*!< The pointer of the log */
} Log_Info_t;

/**
 * @brief Initialization of log
 * 
 */
void app_func_logs_init(void);

/**
 * @brief Write the event to the log
 * 
 * @param event_type The type of event
 * @param callback Callback after writing event
 */
void app_func_logs_event_write(const char* event_type, Log_Event_Write_Callback callback);

/**
 * @brief Write battery voltage to log
 * 
 * @param vbatA The voltage of battery 1
 * @param vbatB The voltage of battery 2
 */
void app_func_logs_batt_volt_write(uint16_t vbatA, uint16_t vbatB);

/**
 * @brief Write impedance to log
 *
 * @param imp The impedance calculated
 */
void app_func_logs_imped_write(uint32_t imp);

/**
 * @brief Write the update of parameters to the log
 * 
 * @param p_id The ID of the parameter
 * @param data_format The type of parameter data.
 * @param p_data Parameter data
 * @param data_len The data length of the parameter
 */
void app_func_logs_parameter_write(uint8_t* p_id, uint8_t data_format, const uint8_t* p_data, uint16_t data_len);

/**
 * @brief Search the logs for this event
 * 
 * @param event_type The type of event
 * @return true This event exists in the log
 * @return false This event does not exist in the log
 */
bool app_func_logs_event_search(const char* event_type);

/**
 * @brief Read a log after this timestamp
 * 
 * @param p_timestamp The timestamp used to search logs
 * @param p_data The log data read.
 * @return uint8_t The length of the log data read
 */
uint8_t app_func_logs_read(const uint8_t* p_timestamp, uint8_t* p_data);

/**
 * @brief Erase log data
 * 
 */
void app_func_logs_erase(void);

/**
 * @brief Log writing completion callback
 *
 * @param write_addr The address of the data to write
 * @param write_size The size of the data to write
 */
void app_func_logs_write_cplt_cb(uint32_t write_addr, uint16_t write_size);

#endif /* FUNCTIONS_INC_APP_FUNC_LOGS_H_ */
