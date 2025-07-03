/**
 * @file app_func_logs.c
 * @brief This file provides log management functions
 * @copyright Copyright (c) 2024
 */
#include "app_func_logs.h"
#include "app_config.h"

#define DATA_TYPE_EVENT					"<EV>"		/*!< Label message for data type "EVENT" */
#define DATA_TYPE_BATT_VOLT				"<BA>"		/*!< Label message for data type "BATT_VOLT" */
#define DATA_TYPE_PARAMETER				"<PA>"		/*!< Label message for data type "PARAMETER" */
#define DATA_TYPE_IMPEDANCE				"<IM>"		/*!< Label message for data type "IMPEDANCE" */

#define PATTERN_TIMESTAMP				"[20YY-MM-DDThh:mm:ssZ(uuu)]"	//UTC format (www.utctime.net)

Log_Info_t logInfo = {
		.LogPointer = ADDR_LOG_BASE,
};

char log_buff_write[512];
char log_buff_read[512];

const char log_end[] = "\r\n";

static uint32_t lastReadAddress = ADDR_LOG_BASE;
static char lastReadTimeStamp[] = PATTERN_TIMESTAMP;

/**
 * @brief Generate timestamp string
 *
 * @param p_timestamp The timestamp used to generate the string. If null, the real time is used
 * @return uint16_t The length of the generated string
 */
static uint16_t app_func_logs_timestamp_gen(const uint8_t* p_timestamp) {
	RTC_TimeTypeDef curr_time;
	RTC_DateTypeDef curr_date;
	HAL_ERROR_CHECK(HAL_RTC_GetTime(&hrtc, &curr_time, RTC_FORMAT_BIN));
	HAL_ERROR_CHECK(HAL_RTC_GetDate(&hrtc, &curr_date, RTC_FORMAT_BIN));
	uint8_t timestamp[] = {
			curr_date.Year,
			curr_date.Month,
			curr_date.Date,
			curr_time.Hours,
			curr_time.Minutes,
			curr_time.Seconds,
			(255U - (uint8_t)curr_time.SubSeconds),
	};
	char* p_log_buff;
	if (p_timestamp != NULL) {
		(void)memcpy(timestamp, p_timestamp, sizeof(timestamp));
		p_log_buff = log_buff_read;
	}
	else {
		(void)memset(log_buff_write, 0, sizeof(log_buff_write));
		p_log_buff = log_buff_write;
	}

	char str_pattern[] = PATTERN_TIMESTAMP;
	char keyword[] = "YMDhms";

	for(uint8_t i = 0;i < sizeof(timestamp);i++) {
		uint8_t posi = (uint8_t)strcspn(str_pattern, &keyword[i]);
		str_pattern[posi] 		= '0' + (timestamp[i] / 10U);
		str_pattern[posi+1U] 	= '0' + (timestamp[i] % 10U);
	}
	str_pattern[22] = '0' + (timestamp[6] / 100U);
	str_pattern[23] = '0' + (timestamp[6] % 100U / 10U);
	str_pattern[24] = '0' + (timestamp[6] % 10U);

	(void)memcpy(p_log_buff, str_pattern, sizeof(str_pattern));
	return (uint16_t)(sizeof(str_pattern) - 1U);
}

/**
 * @brief Write logs to FRAM
 * 
 * @param str The string of logs
 * @param len_str The length of string of logs
 */
static void app_func_logs_write(char* str, uint16_t len_str) {
	uint8_t* p_log = (uint8_t*)str;

	if ((logInfo.LogPointer + len_str) >= (ADDR_LOG_BASE + SIZE_LOG)) {
		logInfo.LogPointer = ADDR_LOG_BASE;
	}

	bsp_fram_write(logInfo.LogPointer, p_log, len_str, false);
}

/**
 * @brief Initialization of log
 * 
 */
void app_func_logs_init(void) {
	bsp_fram_read(ADDR_LOG_INFO, (uint8_t*)&logInfo, sizeof(logInfo));
	if ((logInfo.LogPointer < ADDR_LOG_BASE) || (logInfo.LogPointer >= (ADDR_LOG_BASE + SIZE_LOG))) {
		logInfo.LogPointer = ADDR_LOG_BASE;
		bsp_fram_write(ADDR_LOG_INFO, (uint8_t*)&logInfo, sizeof(logInfo), false);
	}
}

/**
 * @brief Write the event to the log
 * 
 * @param event_type The type of event
 * @param callback Callback after writing event
 */
void app_func_logs_event_write(const char* event_type, Log_Event_Write_Callback callback) {
	uint16_t offset = app_func_logs_timestamp_gen(NULL);

	(void)memcpy(&log_buff_write[offset], DATA_TYPE_EVENT, LEN_DATA_TYPE_STR);
	offset += LEN_DATA_TYPE_STR;

	(void)memcpy(&log_buff_write[offset], event_type, LEN_EVENT_TYPE_STR);
	offset += LEN_EVENT_TYPE_STR;

	(void)memcpy(&log_buff_write[offset], log_end, sizeof(log_end));
	offset += (uint16_t)(sizeof(log_end));

	app_func_logs_write(log_buff_write, offset);
}

/**
 * @brief Write battery voltage to log
 * 
 * @param vbatA The voltage of battery 1
 * @param vbatB The voltage of battery 2
 */
void app_func_logs_batt_volt_write(uint16_t vbatA, uint16_t vbatB) {
	uint16_t offset = app_func_logs_timestamp_gen(NULL);

	(void)memcpy(&log_buff_write[offset], DATA_TYPE_BATT_VOLT, LEN_DATA_TYPE_STR);
	offset += LEN_DATA_TYPE_STR;

	char str_pattern[] = "A=X.XV, B=Y.YV";
	char keyword[] = "XY";
	uint8_t posi = (uint8_t)strcspn(str_pattern, &keyword[0]);
	str_pattern[posi] 		= '0' + (vbatA % 10000U / 1000U);
	//'.'
	str_pattern[posi+2U] 	= '0' + (vbatA % 1000U / 100U);

	posi = (uint8_t)strcspn(str_pattern, &keyword[1]);
	str_pattern[posi] 		= '0' + (vbatB % 10000U / 1000U);
	//'.'
	str_pattern[posi+2U] 	= '0' + (vbatB % 1000U / 100U);

	(void)memcpy(&log_buff_write[offset], str_pattern, strlen(str_pattern));
	offset += (uint16_t)(strlen(str_pattern));

	(void)memcpy(&log_buff_write[offset], log_end, sizeof(log_end));
	offset += (uint16_t)(sizeof(log_end));

	app_func_logs_write(log_buff_write, offset);
}

/**
 * @brief Write impedance to log
 *
 * @param imp The impedance calculated
 */
void app_func_logs_imped_write(uint32_t imp) {
	uint16_t offset = app_func_logs_timestamp_gen(NULL);

	(void)memcpy(&log_buff_write[offset], DATA_TYPE_IMPEDANCE, LEN_DATA_TYPE_STR);
	offset += LEN_DATA_TYPE_STR;

	char str_pattern[] = "X,XXX,XXXohm";
	char keyword[] = "X";
	uint8_t posi = (uint8_t)strcspn(str_pattern, &keyword[0]);
	str_pattern[posi] 		= '0' + (imp % 10000000U / 1000000U);
	//','
	str_pattern[posi+2U] 	= '0' + (imp % 1000000U / 100000U);
	str_pattern[posi+3U] 	= '0' + (imp % 100000U / 10000U);
	str_pattern[posi+4U] 	= '0' + (imp % 10000U / 1000U);
	//','
	str_pattern[posi+6U] 	= '0' + (imp % 1000U / 100U);
	str_pattern[posi+7U] 	= '0' + (imp % 100U / 10U);
	str_pattern[posi+8U] 	= '0' + (imp % 10U);

	uint8_t str_offset = 0;
	while (str_pattern[str_offset] == '0' || str_pattern[str_offset] == ',') {
		str_offset++;
	}

	(void)memcpy(&log_buff_write[offset], &str_pattern[str_offset], strlen(str_pattern)-str_offset);
	offset += (uint16_t)(strlen(str_pattern));

	(void)memcpy(&log_buff_write[offset], log_end, sizeof(log_end));
	offset += (uint16_t)(sizeof(log_end));

	app_func_logs_write(log_buff_write, offset);
}

/**
 * @brief Write the update of parameters to the log
 * 
 * @param p_id The ID of the parameter
 * @param data_format The type of parameter data.
 * @param p_data Parameter data
 * @param data_len The data length of the parameter
 */
void app_func_logs_parameter_write(uint8_t* p_id, uint8_t data_format, const uint8_t* p_data, uint16_t data_len) {
	uint16_t offset = app_func_logs_timestamp_gen(NULL);

	(void)memcpy(&log_buff_write[offset], DATA_TYPE_PARAMETER, LEN_DATA_TYPE_STR);
	offset += LEN_DATA_TYPE_STR;

	char strID[] = "(xPID)";
	(void)memcpy(&strID[1], (char*)p_id, LEN_ID);
	(void)memcpy(&log_buff_write[offset], strID, LEN_ID + 2U);
	offset += (LEN_ID + 2U);

	if (data_format == FORMAT_TYPE_VALUE) {
		_Float64 valx10f = 0.0;
		(void)memcpy((uint8_t*)&valx10f, p_data, sizeof(_Float64));
		valx10f *= 10.0;
		uint16_t valx10 = (uint16_t)valx10f;

		char str_pattern[] = "Val=xxxxxx";
		char keyword[] = "x";
		uint8_t posi = (uint8_t)strcspn(str_pattern, &keyword[0]);
		str_pattern[posi] 		= '0' + (valx10 % 100000U / 10000U);
		str_pattern[posi+1U] 	= '0' + (valx10 % 10000U / 1000U);
		str_pattern[posi+2U] 	= '0' + (valx10 % 1000U / 100U);
		str_pattern[posi+3U] 	= '0' + (valx10 % 100U / 10U);
		str_pattern[posi+4U] 	= '.';
		str_pattern[posi+5U] 	= '0' + (valx10 % 10U);

		(void)memcpy(&log_buff_write[offset], str_pattern, strlen(str_pattern));
		offset += (uint16_t)strlen(str_pattern);
	}
	else {
		char hex[] = "0123456789ABCDEF";
		char num[] = "00";
		for(uint16_t i=0;i<data_len;i++) {
			num[0] = hex[p_data[i] / 16U];
			num[1] = hex[p_data[i] % 16U];
			(void)memcpy(&log_buff_write[offset], num, 2U);
			offset += 2U;
		}
	}
	(void)memcpy(&log_buff_write[offset], log_end, sizeof(log_end));
	offset += (uint16_t)(sizeof(log_end));

	app_func_logs_write(log_buff_write, offset);
}

/**
 * @brief Search the logs for this event
 * 
 * @param event_type The type of event
 * @return true This event exists in the log
 * @return false This event does not exist in the log
 */
bool app_func_logs_event_search(const char* event_type) {
	bool result = false;
	(void)memset(log_buff_write, 0, sizeof(log_buff_write));
	uint16_t offset = 0;
	char* str_event = log_buff_write;

	(void)memcpy(&str_event[offset], DATA_TYPE_EVENT, LEN_DATA_TYPE_STR);
	offset += LEN_DATA_TYPE_STR;

	(void)memcpy(&str_event[offset], event_type, LEN_EVENT_TYPE_STR);
	offset += LEN_EVENT_TYPE_STR;

	for(uint32_t i=0;i<SIZE_LOG;i++) {
		bsp_fram_read(ADDR_LOG_BASE + i, (uint8_t*)log_buff_read, offset);
		if (memcmp((uint8_t*)str_event, (uint8_t*)log_buff_read, offset) == 0) {
			result = true;
			break;
		}
		bsp_wdg_refresh();
	}
	return result;
}

/**
 * @brief Read a log after this timestamp
 * 
 * @param p_timestamp The timestamp used to search logs
 * @param p_data The log data read.
 * @return uint8_t The length of the log data read
 */
uint8_t app_func_logs_read(const uint8_t* p_timestamp, uint8_t* p_data) {
	uint8_t len_read = 0U;
	uint16_t len_timestamp = app_func_logs_timestamp_gen(p_timestamp);
	char* str_timestamp = log_buff_read;
	uint32_t addr_base = logInfo.LogPointer;
	if (memcmp((uint8_t*)lastReadTimeStamp, (uint8_t*)str_timestamp, len_timestamp) == 0) {
		addr_base = lastReadAddress;
	}

	uint32_t addr_pointer = 0U;
	for(uint32_t i=0;i<SIZE_LOG;i++) {
		addr_pointer = ((addr_base + i)%(ADDR_LOG_BASE + SIZE_LOG)) + ADDR_LOG_BASE;
		bsp_fram_read(addr_pointer, p_data, 1);
		if (p_data[0] == '[') {
			bsp_fram_read(addr_pointer, p_data, len_timestamp);
			if (p_data[len_timestamp - 1U] == ']') {
				if (memcmp(p_data, (uint8_t*)str_timestamp, len_timestamp) > 0) {
					len_read = len_timestamp;
					for(uint8_t j=0;j<LEN_RESP_PAYLOAD_MAX;j++) {
						bsp_wdg_refresh();
						bsp_fram_read(addr_pointer + len_read, &p_data[len_read], 1);
						if ((char)p_data[len_read] == '\n' || (char)p_data[len_read] == '\0') {
							memcpy((uint8_t*)lastReadTimeStamp, p_data, len_timestamp);
							lastReadAddress = addr_pointer;
							return len_read + 1U;
						}
						len_read++;
					}
				}
			}
		}
		bsp_wdg_refresh();
	}
	return 0;
}

/**
 * @brief Erase log data
 * 
 */
void app_func_logs_erase(void) {
	bsp_wdg_refresh();
	bsp_fram_erase(ADDR_LOG_BASE, SIZE_LOG);
	bsp_fram_erase(ADDR_LOG_INFO, sizeof(Log_Info_t));
	memset(&logInfo, 0, sizeof(Log_Info_t));
	logInfo.LogPointer = ADDR_LOG_BASE;
}

/**
 * @brief Log writing completion callback
 *
 * @param write_addr The address of the data to write
 * @param write_size The size of the data to write
 */
void app_func_logs_write_cplt_cb(uint32_t write_addr, uint16_t write_size) {
	if ((write_addr >= ADDR_LOG_BASE) && (write_addr < (ADDR_LOG_BASE + SIZE_LOG))) {
		logInfo.LogPointer += ((uint32_t)write_size - 1U);
		bsp_fram_write(ADDR_LOG_INFO, (uint8_t*)&logInfo, sizeof(logInfo), false);
	}
}
