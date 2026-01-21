/**
 * @file app_cmd.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief All command applications
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef APP_CMD_H_
#define APP_CMD_H_

#include <stdint.h>
#include "ble_nus.h"

#define OPCODE_BLE_STAT_GET                         0x50
#define OPCODE_BLE_ADV_START                        0x51
#define OPCODE_BLE_ADV_STOP                         0x52
#define OPCODE_BLE_DISCONNECT                       0x53
#define OPCODE_BLE_WL_ADD                           0x54
#define OPCODE_BLE_DEL_PEERS                        0x55

#define STATUS_SUCCESS                              0x00
#define STATUS_INVALID                              0x01

#define STATUS_CRC_ERR                              0xF0
#define STATUS_PAYLOAD_LEN_ERR                      0xF1
#define STATUS_OPCODE_ERR                           0xF2

#define REQ_HEADER_LEN                              2
#define RESP_HEADER_LEN                             REQ_HEADER_LEN + 1
#define CRC_LEN                                     2

typedef struct
{
    uint16_t cmd_resp_len;
    uint8_t cmd_resp[BLE_NUS_MAX_DATA_LEN];
} app_cmd_resp_t;

/**
 * @brief Parse the request command
 * 
 * @param p_cmd Pointer to the command to parse
 * @param cmd_len The length of the command
 * @return app_cmd_resp_t* The response command of request command
 */
app_cmd_resp_t* app_cmd_parse_request(uint8_t* p_cmd, uint16_t cmd_len);

#endif