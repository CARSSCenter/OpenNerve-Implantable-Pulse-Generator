/**
 * @file app_func_command.c
 * @brief This file provides command management functions
 * @copyright Copyright (c) 2024
 */
#include "app_func_command.h"
#include "app_config.h"

static Cmd_Req_Parser 	curr_cmd_req_parser = NULL;
static Cmd_Resp_Parser	curr_cmd_resp_parser = NULL;
static uint32_t crc_buffer[LEN_CMD_MAX/sizeof(uint32_t)];
Cmd_Resp_t 	cmd_resp;
Cmd_Req_t 	cmd_req;

/**
 * @brief Confirm the CRC value of the command
 * 
 * @param p_cmd Command to confirm CRC value
 * @param cmd_len The length of the command to confirm the CRC value
 * @return true Confirmation is passed
 * @return false Confirmation is failed
 */
static bool app_func_command_crc_confirm(const uint8_t* p_cmd, uint8_t cmd_len) {
	bool result = false;
	uint16_t cmd_crc16 = 0;

	(void)memcpy((uint8_t*)crc_buffer, p_cmd, (uint32_t)cmd_len - LEN_CRC);
	(void)memcpy((uint8_t*)&cmd_crc16, &p_cmd[cmd_len - LEN_CRC], LEN_CRC);
    uint16_t cal_crc16 = (uint16_t)HAL_CRC_Calculate(&hcrc, crc_buffer, (uint32_t)cmd_len - LEN_CRC);
    if (cmd_crc16 == cal_crc16) {
    	result = true;
    }
    else {
    	result = false;
    }
    return result;
}

/**
 * @brief Parser for response commands, used to control the BLE chip
 * 
 * @param p_cmd_resp Response command to be parsed
 * @param cmd_resp_len The length of the response command to be parsed
 */
static void app_func_command_resp_parser(uint8_t* p_cmd_resp, uint8_t cmd_resp_len) {
	UNUSED(cmd_resp_len);
	if (curr_cmd_resp_parser != NULL) {
		cmd_resp.Opcode = p_cmd_resp[0];
		cmd_resp.Status = p_cmd_resp[2];
		cmd_resp.Payload = NULL;
		cmd_resp.PayloadLen = p_cmd_resp[1];
		if (cmd_resp.PayloadLen > 0) {
			cmd_resp.Payload = &p_cmd_resp[LEN_RESP_HEADER];
		}
		curr_cmd_resp_parser(cmd_resp);
	}
}

/**
 * @brief Parser for request commands, used to communicate with the remote end
 * 
 * @param p_cmd_req Request command to be parsed
 * @param cmd_req_len The length of the request command to be parsed
 * @param p_cmd_resp The response command to be replied after parsing the request command
 * @return uint8_t The length of the reply response command
 */
static uint8_t app_func_command_req_parser(uint8_t* p_cmd_req, uint8_t cmd_req_len, uint8_t* p_cmd_resp) {
	UNUSED(cmd_req_len);
	cmd_resp.Opcode = p_cmd_req[0];
	cmd_resp.Status = STATUS_OPCODE_ERR;
	cmd_resp.Payload = NULL;
	cmd_resp.PayloadLen = 0;

	if (curr_cmd_req_parser != NULL) {
		cmd_req.Opcode = p_cmd_req[0];
		cmd_req.Payload = NULL;
		cmd_req.PayloadLen = p_cmd_req[1];
		if (cmd_req.PayloadLen > 0) {
			cmd_req.Payload = &p_cmd_req[LEN_REQ_HEADER];
		}
		cmd_resp = curr_cmd_req_parser(cmd_req);
	}

    uint8_t cmd_resp_len = LEN_RESP_HEADER + cmd_resp.PayloadLen + LEN_CRC;
    p_cmd_resp[0] = cmd_resp.Opcode;
    p_cmd_resp[1] = cmd_resp.PayloadLen;
    p_cmd_resp[2] = cmd_resp.Status;
    if ((cmd_resp.PayloadLen > 0U) && (cmd_resp.Payload != NULL)) {
        (void)memcpy(&p_cmd_resp[LEN_RESP_HEADER], cmd_resp.Payload, cmd_resp.PayloadLen);
    }

	(void)memcpy((uint8_t*)crc_buffer, p_cmd_resp, (uint32_t)cmd_resp_len - LEN_CRC);
    uint16_t cal_crc16 = (uint16_t)HAL_CRC_Calculate(&hcrc, crc_buffer, (uint32_t)cmd_resp_len - LEN_CRC);
    (void)memcpy(&p_cmd_resp[cmd_resp_len - LEN_CRC], (uint8_t*)&cal_crc16, LEN_CRC);
    return cmd_resp_len;
}

/**
 * @brief Update parser for request commands
 * 
 * @param new_cmd_req_parser New request command parser
 */
void app_func_command_req_parser_set(Cmd_Req_Parser new_cmd_req_parser) {
	curr_cmd_req_parser = new_cmd_req_parser;
}

/**
 * @brief Update parser for response commands
 *
 * @param new_cmd_resp_parser New response command parser
 */
void app_func_command_resp_parser_set(Cmd_Resp_Parser new_cmd_resp_parser) {
	curr_cmd_resp_parser = new_cmd_resp_parser;
}

/**
 * @brief Parser for all commands, used to confirm whether the command is a request command or a response command
 * 
 * @param p_data_rx The data received
 * @param p_data_rx_len The length of data received
 * @param p_data_tx The data to be transferred
 * @return uint8_t The length of data to be transferred
 */
uint8_t app_func_command_parser(uint8_t* p_data_rx, uint8_t* p_data_rx_len, uint8_t* p_data_tx) {
	uint8_t data_tx_len = 0;
	uint8_t cmd_len = 0;
	uint8_t payload_length = p_data_rx[1];

	if (app_func_command_crc_confirm(p_data_rx, LEN_RESP_HEADER + payload_length + LEN_CRC)) {
		cmd_len = (LEN_RESP_HEADER + payload_length + LEN_CRC);
		app_func_command_resp_parser(p_data_rx, cmd_len);
		(void)memmove(p_data_rx, &p_data_rx[cmd_len], (size_t)SP_BUF_SIZE - cmd_len);
		*p_data_rx_len -= cmd_len;
	}
	else if (app_func_command_crc_confirm(p_data_rx, LEN_REQ_HEADER + payload_length + LEN_CRC)) {
		cmd_len = (LEN_REQ_HEADER + payload_length + LEN_CRC);
		data_tx_len = app_func_command_req_parser(p_data_rx, cmd_len, p_data_tx);
		(void)memmove(p_data_rx, &p_data_rx[cmd_len], (size_t)SP_BUF_SIZE - cmd_len);
		*p_data_rx_len -= cmd_len;
	}
	else {
		p_data_tx[0] = p_data_rx[0];
		p_data_tx[1] = 0;
		p_data_tx[2] = STATUS_CRC_ERR;

		(void)memcpy((uint8_t*)crc_buffer, p_data_tx, (uint32_t)LEN_RESP_HEADER);
	    uint16_t cal_crc16 = (uint16_t)HAL_CRC_Calculate(&hcrc, crc_buffer, LEN_RESP_HEADER);
	    (void)memcpy(&p_data_tx[LEN_RESP_HEADER], (uint8_t*)&cal_crc16, LEN_CRC);
	    data_tx_len = LEN_RESP_HEADER + LEN_CRC;
	    *p_data_rx_len = 0;
	}
	return data_tx_len;
}

/**
 * @brief Generate and send a request command
 * 
 * @param cmd The definition of the request command to be generated
 */
void app_func_command_req_send(Cmd_Req_t cmd) {
	uint8_t req_cmd[LEN_CMD_MAX];
	req_cmd[0] = cmd.Opcode;
	req_cmd[1] = cmd.PayloadLen;

    if ((cmd.PayloadLen > 0U) && (cmd.Payload != NULL)) {
        (void)memcpy(&req_cmd[LEN_REQ_HEADER], cmd.Payload, cmd.PayloadLen);
    }

	(void)memcpy((uint8_t*)crc_buffer, req_cmd, (uint32_t)LEN_REQ_HEADER + cmd.PayloadLen);
    uint16_t cal_crc16 = (uint16_t)HAL_CRC_Calculate(&hcrc, crc_buffer, (uint32_t)LEN_REQ_HEADER + cmd.PayloadLen);
    (void)memcpy(&req_cmd[LEN_REQ_HEADER + cmd.PayloadLen], (uint8_t*)&cal_crc16, LEN_CRC);
    bsp_sp_cmd_send(req_cmd, LEN_REQ_HEADER + cmd.PayloadLen + LEN_CRC);
}

/**
 * @brief Generate and send a response command
 *
 * @param cmd The definition of the response command to be generated
 */
void app_func_command_resp_send(Cmd_Resp_t cmd) {
	uint8_t resp_cmd[LEN_CMD_MAX];
	resp_cmd[0] = cmd.Opcode;
	resp_cmd[1] = cmd.PayloadLen;
	resp_cmd[2] = cmd.Status;

    if ((cmd.PayloadLen > 0U) && (cmd.Payload != NULL)) {
        (void)memcpy(&resp_cmd[LEN_RESP_HEADER], cmd.Payload, cmd.PayloadLen);
    }

	(void)memcpy((uint8_t*)crc_buffer, resp_cmd, (uint32_t)LEN_RESP_HEADER + cmd.PayloadLen);
    uint16_t cal_crc16 = (uint16_t)HAL_CRC_Calculate(&hcrc, crc_buffer, (uint32_t)LEN_RESP_HEADER + cmd.PayloadLen);
    (void)memcpy(&resp_cmd[LEN_RESP_HEADER + cmd.PayloadLen], (uint8_t*)&cal_crc16, LEN_CRC);
    bsp_sp_cmd_send(resp_cmd, LEN_RESP_HEADER + cmd.PayloadLen + LEN_CRC);
}

