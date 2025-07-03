/**
 * @file app_mode_oad.c
 * @brief This file provides management of the OAD mode
 * @copyright Copyright (c) 2024
 */
#include "app_mode_oad.h"
#include "app_config.h"

/**
 * @brief Parser for request commands in OAD mode, used to communicate with the remote end
 * 
 * @param req Request command to be parsed
 * @return Cmd_Resp_t The response command to be replied after parsing the request command
 */
static Cmd_Resp_t app_mode_oad_cmd_parser(Cmd_Req_t req) {
	Cmd_Resp_t resp = {
			.Opcode 		= req.Opcode,
			.Status 		= STATUS_SUCCESS,
			.Payload 		= NULL,
			.PayloadLen 	= 0,
	};
	uint8_t len_payload_min = 0;
	uint8_t len_payload_max = 0;

	switch(req.Opcode) {
	case OP_DOWNLOAD_FW_IMAGE:
	{
		len_payload_min = (uint8_t)sizeof(FW_Image_Packet_t);
		len_payload_max = (uint8_t)sizeof(FW_Image_Packet_t);
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			FW_Image_Packet_t image_packet_write;
			(void)memcpy((uint8_t*)&image_packet_write, req.Payload, sizeof(FW_Image_Packet_t));
			if ((image_packet_write.ImageDataOffset + SIZE_FW_IMG_PKG) > SIZE_FW_IMG) {
				resp.Status = STATUS_INVALID;
			}
			else {
				FW_Image_Packet_t image_packet_read;
				(void)memcpy((uint8_t*)&image_packet_read, req.Payload, sizeof(FW_Image_Packet_t));
				bsp_fram_read(ADDR_FW_IMG_BASE + image_packet_read.ImageDataOffset, image_packet_read.ImageData, SIZE_FW_IMG_PKG);

				if (memcmp(image_packet_read.ImageData, image_packet_write.ImageData, sizeof(image_packet_read.ImageData)) != 0) {
					bsp_fram_write(ADDR_FW_IMG_BASE + image_packet_write.ImageDataOffset, image_packet_write.ImageData, SIZE_FW_IMG_PKG, true);
				}
			}
		}
	}
		break;

	case OP_VERIFY_FW_IMAGE:
	{
		len_payload_min = (uint8_t)sizeof(uint32_t);
		len_payload_max = (uint8_t)sizeof(uint32_t);
		if ((req.PayloadLen < len_payload_min) || (req.PayloadLen > len_payload_max)) {
			resp.Status = STATUS_PAYLOAD_LEN_ERR;
		}
		else {
			uint32_t image_size = 1U;
			(void)memcpy((uint8_t*)&image_size, req.Payload, sizeof(image_size));
			uint8_t* p_fail_num = app_func_auth_compare_fram_hash(image_size);
			if (*p_fail_num == 0U) {
				app_func_sm_current_state_set(STATE_ACT_MODE_BSL);
			}
			else {
				resp.Status = STATUS_INVALID;
				resp.PayloadLen = (uint8_t)sizeof(uint8_t);
				resp.Payload = p_fail_num;

				if (*p_fail_num >= 3U) {
					app_func_sm_current_state_set(STATE_ACT_MODE_BLE_CONN);
				}
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
 * @brief Handler for OAD mode
 * 
 */
void app_mode_oad_handler(void) {
	uint16_t curr_state = app_func_sm_current_state_get();
	app_func_command_req_parser_set(&app_mode_oad_cmd_parser);
	uint8_t curr_ble_state = app_func_ble_curr_state_get();
	uint8_t cmd_counter = 0xFF;

	while(curr_state == STATE_ACT_MODE_OAD) {
		bsp_wdg_refresh();
		if (cmd_counter == 0) {
			app_func_ble_new_state_get();
			cmd_counter = 0xFF;
		}
		bsp_sp_cmd_handler();
		HAL_Delay(50);
		curr_ble_state = app_func_ble_curr_state_get();
		curr_state = app_func_sm_current_state_get();
		if (curr_state != STATE_ACT_MODE_OAD) {
			bsp_wdg_refresh();
			bsp_sp_cmd_handler();
			HAL_Delay(100);
		}
		else if (curr_ble_state == BLE_STATE_ADV_STOP) {
			app_func_sm_current_state_set(STATE_ACT);
			curr_state = app_func_sm_current_state_get();
		}
		else {
			__NOP();
		}
		cmd_counter--;
	}
}
