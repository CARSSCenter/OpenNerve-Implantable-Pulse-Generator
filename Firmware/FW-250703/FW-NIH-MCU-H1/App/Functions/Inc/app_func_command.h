/**
 * @file app_func_command.h
 * @brief This file contains all the function prototypes for the app_func_command.c file
 * @copyright Copyright (c) 2024
 */
#ifndef FUNCTIONS_INC_APP_FUNC_COMMAND_H_
#define FUNCTIONS_INC_APP_FUNC_COMMAND_H_
#include <stdint.h>

//BLE Commands
#define OP_BLE_STAT_GET       						0x50U	/*!< The opcode of the command "BLE_STAT_GET" */
#define OP_BLE_ADV_START      						0x51U	/*!< The opcode of the command "BLE_ADV_START" */
#define OP_BLE_ADV_STOP       						0x52U	/*!< The opcode of the command "BLE_ADV_STOP" */
#define OP_BLE_DISCONNECT     						0x53U	/*!< The opcode of the command "BLE_DISCONNECT" */
#define OP_BLE_WL_ADD         						0x54U	/*!< The opcode of the command "BLE_WL_ADD" */
#define OP_BLE_DEL_PEERS                        	0x55U	/*!< The opcode of the command "BLE_DEL_PEERS" */

//SYS Commands
#define OP_AUTH                        				0xF0U	/*!< The opcode of the command "AUTH" */
#define OP_READ_BLE_ADVANCE            				0xF1U	/*!< The opcode of the command "READ_BLE_ADVANCE" */
#define OP_WRITE_BLE_ADVANCE           				0xF2U	/*!< The opcode of the command "WRITE_BLE_ADVANCE" */
#define OP_AUTH_FW_IMAGE               				0xF3U	/*!< The opcode of the command "AUTH_FW_IMAGE" */
#define OP_DOWNLOAD_FW_IMAGE						0xF4U	/*!< The opcode of the command "DOWNLOAD_FW_IMAGE" */
#define OP_VERIFY_FW_IMAGE							0xF5U	/*!< The opcode of the command "VERIFY_FW_IMAGE" */
#define OP_SET_START_STATE							0xF6U	/*!< The opcode of the command "SET_START_STATE" */

//IPG Commands
#define OP_SHUTDOWN_SYSTEM             				0xA0U	/*!< The opcode of the command "SHUTDOWN_SYSTEM" */
#define OP_REBOOT_SYSTEM               				0xA1U	/*!< The opcode of the command "REBOOT_SYSTEM" */
#define OP_BLE_DISCONNECT_REQUEST     				0xA2U	/*!< The opcode of the command "BLE_DISCONNECT_REQUEST" */
#define OP_START_SCHED_THERAPY_SESSION      		0xA3U	/*!< The opcode of the command "START_SCHEDULED_THERAPY_SESSION" */
#define OP_END_SCHED_THERAPY_SESSION        		0xA4U	/*!< The opcode of the command "END_SCHEDULED_THERAPY_SESSION" */
#define OP_START_MANUAL_THERAPY_SESSION       		0xA5U	/*!< The opcode of the command "START_MANUAL_THERAPY_SESSION" */
#define OP_STOP_MANUAL_THERAPY_SESSION        		0xA6U	/*!< The opcode of the command "STOP_MANUAL_THERAPY_SESSION" */
#define OP_MEASURE_IMPEDANCE            			0xA7U	/*!< The opcode of the command "MEASURE_IMPEDANCE" */
#define OP_MEASURE_BATTERY_VOLTAGE 	   				0xA8U	/*!< The opcode of the command "MEASURE_BATTERY_VOLTAGE" */
#define OP_MEASURE_SENSOR_VOLTAGE       			0xA9U	/*!< The opcode of the command "MEASURE_SENSOR_VOLTAGE" */
#define OP_READ_HARDWARE_PARAMETERS    	 			0xAAU	/*!< The opcode of the command "READ_HARDWARE_PARAMETERS" */
#define OP_WRITE_HARDWARE_PARAMETERS    			0xABU	/*!< The opcode of the command "WRITE_HARDWARE_PARAMETERS" */
#define OP_READ_STIMULATION_PARAMETERS  			0xACU	/*!< The opcode of the command "READ_STIMULATION_PARAMETERS" */
#define OP_WRITE_STIMULATION_PARAMETERS 			0xADU	/*!< The opcode of the command "WRITE_STIMULATION_PARAMETERS" */
#define OP_READ_IPG_LOG                 			0xAEU	/*!< The opcode of the command "READ_IPG_LOG" */
#define OP_ERASE_IPG_LOG                			0xAFU	/*!< The opcode of the command "ERASE_IPG_LOG" */
#define OP_READ_TIME_AND_DATE           			0xB0U	/*!< The opcode of the command "READ_TIME_AND_DATE" */
#define OP_WRITE_TIME_AND_DATE          			0xB1U	/*!< The opcode of the command "WRITE_TIME_AND_DATE" */

//DVT Commands
#define OP_PING                                    	0x00U	/*!< The opcode of the command "PING" */
#define OP_GET_TEST_INFORMATION                    	0x01U	/*!< The opcode of the command "GET_TEST_INFORMATION" */
#define OP_SET_SAMPLE_ID                           	0x02U	/*!< The opcode of the command "SET_SAMPLE_ID" */
#define OP_GET_SAMPLE_ID                           	0x03U	/*!< The opcode of the command "GET_SAMPLE_ID" */
#define OP_SET_DVT_MODE                            	0x04U	/*!< The opcode of the command "SET_DVT_MODE" */
#define OP_TURN_ON_HV_SUPPLY                       	0x05U	/*!< The opcode of the command "TURN_ON_HV_SUPPLY" */
#define OP_SET_HV_SUPPLY_VOLTAGE_VALUE             	0x06U	/*!< The opcode of the command "SET_HV_SUPPLY_VOLTAGE_VALUE" */
#define OP_ENABLE_HV_SUPPLY                        	0x07U	/*!< The opcode of the command "ENABLE_HV_SUPPLY" */
#define OP_IPG_SHUTDOWN                            	0x08U	/*!< The opcode of the command "IPG_SHUTDOWN" */
#define OP_ENABLE_VDDS_SUPPLY                      	0x09U	/*!< The opcode of the command "ENABLE_VDDS_SUPPLY" */
#define OP_ENABLE_VDDA_SUPPLY                      	0x0AU	/*!< The opcode of the command "ENABLE_VDDA_SUPPLY" */
#define OP_ENABLE_BATTERY_MONITOR                  	0x0BU	/*!< The opcode of the command "ENABLE_BATTERY_MONITOR" */
#define OP_GET_BATTERY_VOLTAGE_MEASUREMENT         	0x0CU	/*!< The opcode of the command "GET_BATTERY_VOLTAGE_MEASUREMENT" */
#define OP_SET_STIMULUS_CIRCUIT_PARAMETERS         	0x0DU	/*!< The opcode of the command "SET_STIMULUS_CIRCUIT_PARAMETERS" */
#define OP_GET_STIMULUS_CIRCUIT_PARAMETERS         	0x0EU	/*!< The opcode of the command "GET_STIMULUS_CIRCUIT_PARAMETERS" */
#define OP_SET_DAC_AB_OUTPUT_VOLTAGE               	0x0FU	/*!< The opcode of the command "SET_DAC_AB_OUTPUT_VOLTAGE" */
#define OP_GENERATE_PHI_ONE_SINGLE_PULSE           	0x10U	/*!< The opcode of the command "GENERATE_PHI_ONE_SINGLE_PULSE" */
#define OP_GENERATE_PHI_TWO_SINGLE_PULSE           	0x11U	/*!< The opcode of the command "GENERATE_PHI_TWO_SINGLE_PULSE" */
#define OP_ENABLE_STIMULUS_CIRCUIT                 	0x12U	/*!< The opcode of the command "ENABLE_STIMULUS_CIRCUIT" */
#define OP_SET_CURRENT_SOURCES_CONFIGURATION       	0x13U	/*!< The opcode of the command "SET_CURRENT_SOURCES_CONFIGURATION" */
#define OP_GET_CURRENT_SOURCES_CONFIGURATION       	0x14U	/*!< The opcode of the command "GET_CURRENT_SOURCES_CONFIGURATION" */
#define OP_GENERATE_MOCKED_STIMULUS                	0x15U	/*!< The opcode of the command "GENERATE_MOCKED_STIMULUS" */
#define OP_ENABLE_CHANNEL_MUX                      	0x16U	/*!< The opcode of the command "ENABLE_CHANNEL_MUX" */
#define OP_SET_SR_SEL_POSITIONS                    	0x17U	/*!< The opcode of the command "SET_SR_SEL_POSITIONS" */
#define OP_GET_SR_SEL_POSITIONS                    	0x18U	/*!< The opcode of the command "GET_SR_SEL_POSITIONS" */
#define OP_DISABLE_ECG_HR_AFE                      	0x19U	/*!< The opcode of the command "DISABLE_ECG_HR_AFE" */
#define OP_DISABLE_ECG_RR_AFE                      	0x1AU	/*!< The opcode of the command "DISABLE_ECG_RR_AFE" */
#define OP_ENABLE_ECG_HR_AFE                       	0x1BU	/*!< The opcode of the command "ENABLE_ECG_HR_AFE" */
#define OP_ENABLE_ECG_RR_AFE                       	0x1CU	/*!< The opcode of the command "ENABLE_ECG_RR_AFE" */
#define OP_GET_ECG_HR_LOD                          	0x1DU	/*!< The opcode of the command "GET_ECG_HR_LOD" */
#define OP_GET_ECG_RR_LOD                          	0x1EU	/*!< The opcode of the command "GET_ECG_RR_LOD" */
#define OP_GET_ECG_HR_AFE_OUTPUT                   	0x1FU	/*!< The opcode of the command "GET_ECG_HR_AFE_OUTPUT" */
#define OP_GET_ECG_RR_AFE_OUTPUT                   	0x20U	/*!< The opcode of the command "GET_ECG_RR_AFE_OUTPUT" */
#define OP_ENABLE_WPT_VRECT_MON_CIRCUIT            	0x21U	/*!< The opcode of the command "ENABLE_WPT_VRECT_MON_CIRCUIT" */
#define OP_DISABLE_WPT_VRECT_MON_CIRCUIT           	0x22U	/*!< The opcode of the command "DISABLE_WPT_VRECT_MON_CIRCUIT" */
#define OP_GET_WPT_VRECT_MON_CIRCUIT_OUTPUT        	0x23U	/*!< The opcode of the command "GET_WPT_VRECT_MON_CIRCUIT_OUTPUT" */
#define OP_GET_VRECT_DET                           	0x24U	/*!< The opcode of the command "GET_VRECT_DET" */
#define OP_GET_VRECT_OVP                           	0x25U	/*!< The opcode of the command "GET_VRECT_OVP" */
#define OP_GET_VCHG_RAIL_SUPPLY_CIRCUIT_POWER_GOOD 	0x26U	/*!< The opcode of the command "GET_VCHG_RAIL_SUPPLY_CIRCUIT_POWER_GOOD" */
#define OP_ENABLE_CHARGE_CONTROL_1_CIRCUIT         	0x27U	/*!< The opcode of the command "ENABLE_CHARGE_CONTROL_1_CIRCUIT" */
#define OP_DISABLE_CHARGE_CONTROL_1_CIRCUIT        	0x28U	/*!< The opcode of the command "DISABLE_CHARGE_CONTROL_1_CIRCUIT" */
#define OP_CHARGE_RATE_CONTROL                     	0x29U	/*!< The opcode of the command "CHARGE_RATE_CONTROL" */
#define OP_GET_CHG1_STATUS                         	0x2AU	/*!< The opcode of the command "GET_CHG1_STATUS" */
#define OP_GET_CHG1_OVP_ERR                        	0x2BU	/*!< The opcode of the command "GET_CHG1_OVP_ERR" */
#define OP_ENABLE_CHARGE_CONTROL_2_CIRCUIT         	0x2CU	/*!< The opcode of the command "ENABLE_CHARGE_CONTROL_2_CIRCUIT" */
#define OP_DISABLE_CHARGE_CONTROL_2_CIRCUIT        	0x2DU	/*!< The opcode of the command "DISABLE_CHARGE_CONTROL_2_CIRCUIT" */
#define OP_GET_CHG2_STATUS                         	0x2EU	/*!< The opcode of the command "GET_CHG2_STATUS" */
#define OP_GET_CHG2_OVP_ERR                        	0x2FU	/*!< The opcode of the command "GET_CHG2_OVP_ERR" */
#define OP_ENABLE_THERMISTOR_INTERFACE_CIRCUIT     	0x30U	/*!< The opcode of the command "ENABLE_THERMISTOR_INTERFACE_CIRCUIT" */
#define OP_DISABLE_THERMISTOR_INTERFACE_CIRCUIT    	0x31U	/*!< The opcode of the command "DISABLE_THERMISTOR_INTERFACE_CIRCUIT" */
#define OP_GET_THERM_REF                           	0x32U	/*!< The opcode of the command "GET_THERM_REF" */
#define OP_GET_THERM_OUT                           	0x33U	/*!< The opcode of the command "GET_THERM_OUT" */
#define OP_GET_THERM_OFST                          	0x34U	/*!< The opcode of the command "GET_THERM_OFST" */
#define OP_DISABLE_EMG1_AFE                        	0x35U	/*!< The opcode of the command "DISABLE_EMG1_AFE" */
#define OP_DISABLE_EMG2_AFE                        	0x36U	/*!< The opcode of the command "DISABLE_EMG2_AFE" */
#define OP_ENABLE_EMG1_AFE                         	0x37U	/*!< The opcode of the command "ENABLE_EMG1_AFE" */
#define OP_ENABLE_EMG2_AFE                         	0x38U	/*!< The opcode of the command "ENABLE_EMG2_AFE" */
#define OP_GET_EMG1_LOD                            	0x39U	/*!< The opcode of the command "GET_EMG1_LOD" */
#define OP_GET_EMG2_LOD                            	0x3AU	/*!< The opcode of the command "GET_EMG2_LOD" */
#define OP_GET_EMG1_AFE_OUTPUT                     	0x3BU	/*!< The opcode of the command "GET_EMG1_AFE_OUTPUT" */
#define OP_GET_EMG2_AFE_OUTPUT                     	0x3CU	/*!< The opcode of the command "GET_EMG2_AFE_OUTPUT" */
#define OP_GET_MAG_STATUS                          	0x3DU	/*!< The opcode of the command "GET_MAG_STATUS" */
#define OP_ENABLE_IMC                              	0x3EU	/*!< The opcode of the command "ENABLE_IMC" */
#define OP_DISABLE_IMC                             	0x3FU	/*!< The opcode of the command "DISABLE_IMC" */
#define OP_SET_IMP_SEL_POSITIONS                   	0x40U	/*!< The opcode of the command "SET_IMP_SEL_POSITIONS" */
#define OP_GET_IMP_SEL_POSITIONS                   	0x41U	/*!< The opcode of the command "GET_IMP_SEL_POSITIONS" */
#define OP_GET_IMC_MEASURE                         	0x42U	/*!< The opcode of the command "GET_IMC_MEASURE" */
#define OP_DISABLE_HV_SUPPLY                       	0x43U	/*!< The opcode of the command "DISABLE_HV_SUPPLY" */
#define OP_DISABLE_VDDS_SUPPLY                     	0x44U	/*!< The opcode of the command "DISABLE_VDDS_SUPPLY" */
#define OP_DISABLE_VDDA_SUPPLY                     	0x45U	/*!< The opcode of the command "DISABLE_VDDA_SUPPLY" */
#define OP_DISABLE_STIMULUS_CIRCUIT                	0x46U	/*!< The opcode of the command "DISABLE_STIMULUS_CIRCUIT" */
#define OP_START_ACC                				0x47U	/*!< The opcode of the command "START_ACC" */
#define OP_GET_DATA_ACC                				0x48U	/*!< The opcode of the command "GET_DATA_ACC" */
#define OP_STOP_ACC                					0x49U	/*!< The opcode of the command "STOP_ACC" */

//Status of Response Commands
#define STATUS_SUCCESS            					0x00U	/*!< Code for success status */
#define STATUS_INVALID            					0x01U	/*!< Code for invalid status */

#define STATUS_CRC_ERR            					0xF0U	/*!< Code for CRC error status */
#define STATUS_PAYLOAD_LEN_ERR    					0xF1U	/*!< Code for payload length error status */
#define STATUS_OPCODE_ERR            				0xF2U	/*!< Code for opcode error status */
#define STATUS_USER_CLASS_ERR    					0xF3U	/*!< Code for user class error status */

//XL board error status
#define STATUS_START_ACC_BUFFER_OVERFLOW	        0x01U	/*!< Code for buffer overflow */
#define STATUS_START_ACC_COMMUNICTION_ERR	        0x02U	/*!< Code for communication error */
#define STATUS_START_ACC_DEVICE_NOT_READY	        0x03U	/*!< Code for device not ready */
#define STATUS_START_ACC_INVALID_CONFIGURATION      0x04U	/*!< Code for invalid configuration */

#define STATUS_GET_ACC_BUFFER_OVERFLOW	         	0x01U	/*!< Code for buffer overflow */
#define STATUS_GET_ACC_BUFFER_EMPTY		         	0x02U	/*!< Code for buffer Empty */
#define STATUS_GET_ACC_STARTUP_NOT_PERFORMED       	0x03U	/*!< Code for Start-up not performed */
#define STATUS_GET_ACC_COMMUNICTION_ERR       		0x04U	/*!< Code for communication error */

#define LEN_CMD_MAX									244U										/*!< The maximum length of the command */
#define LEN_REQ_HEADER 								2U											/*!< The length of the request command header */
#define LEN_RESP_HEADER 							(LEN_REQ_HEADER + 1U)						/*!< The length of the response command header */
#define LEN_CRC 	 								2U											/*!< The length of the command CRC */
#define LEN_REQ_PAYLOAD_MAX							(LEN_CMD_MAX - LEN_REQ_HEADER - LEN_CRC)	/*!< The maximum length of the request command payload */
#define LEN_RESP_PAYLOAD_MAX						(LEN_CMD_MAX - LEN_RESP_HEADER - LEN_CRC)	/*!< The maximum length of the response command payload */

typedef struct {
	uint8_t Opcode;			/*!< The opcode of the command */
	uint8_t* Payload;		/*!< The payload pointer of the command */
	uint8_t PayloadLen;		/*!< The payload length of the command */
} Cmd_Req_t;

typedef struct {
	uint8_t Opcode;			/*!< The opcode of the command */
	uint8_t Status;			/*!< The status of the response command */
	uint8_t* Payload;		/*!< The payload pointer of the command */
	uint8_t PayloadLen;		/*!< The payload length of the command */
} Cmd_Resp_t;

typedef Cmd_Resp_t (*Cmd_Req_Parser)(Cmd_Req_t cmd_req);	/*!< The format of the request command parser */
typedef void (*Cmd_Resp_Parser)(Cmd_Resp_t cmd_resp);		/*!< The format of the request command parser */

/**
 * @brief Update parser for request commands
 * 
 * @param new_cmd_req_parser New request command parser
 */
void app_func_command_req_parser_set(Cmd_Req_Parser new_cmd_req_parser);

/**
 * @brief Update parser for response commands
 *
 * @param new_cmd_resp_parser New response command parser
 */
void app_func_command_resp_parser_set(Cmd_Resp_Parser new_cmd_resp_parser);

/**
 * @brief Parser for all commands, used to confirm whether the command is a request command or a response command
 * 
 * @param p_data_rx The data received
 * @param p_data_rx_len The length of data received
 * @param p_data_tx The data to be transferred
 * @return uint8_t The length of data to be transferred
 */
uint8_t app_func_command_parser(uint8_t* p_data_rx, uint8_t* p_data_rx_len, uint8_t* p_data_tx);

/**
 * @brief Generate and send a request command
 * 
 * @param cmd The definition of the request command to be generated
 */
void app_func_command_req_send(Cmd_Req_t cmd);

/**
 * @brief Generate and send a response command
 *
 * @param cmd The definition of the response command to be generated
 */
void app_func_command_resp_send(Cmd_Resp_t cmd);

#endif /* FUNCTIONS_INC_APP_FUNC_COMMAND_H_ */
