/**
 * @file app_cmd.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief All command applications
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_cmd.h"
#include "app_ble.h"
#include "app_sp.h"

#include "crc16.h"

#define NRF_LOG_MODULE_NAME APP_CMD
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

static app_cmd_resp_t response;

/**
 * @brief Generate response command
 * 
 * @param opcode The opcode of the response command
 * @param status The status of the response command
 * @param payload Pointer to the payload of the response command
 * @param payload_len The length of the payload
 * @return app_cmd_resp_t* The generated response command
 */
static app_cmd_resp_t* generate_response(uint8_t opcode, uint8_t status, uint8_t* payload, uint8_t payload_len)
{
    response.cmd_resp_len = RESP_HEADER_LEN + payload_len + CRC_LEN;
    response.cmd_resp[0] = opcode;
    response.cmd_resp[1] = payload_len;
    response.cmd_resp[2] = status;

    if (payload_len > 0 && payload != NULL)
        memcpy(response.cmd_resp + RESP_HEADER_LEN, payload, payload_len);

    uint16_t crc16 = crc16_compute(response.cmd_resp, response.cmd_resp_len - CRC_LEN, NULL);
    memcpy(response.cmd_resp + response.cmd_resp_len - CRC_LEN, (uint8_t*)&crc16, CRC_LEN);
    NRF_LOG_HEXDUMP_INFO((uint8_t*)response.cmd_resp, response.cmd_resp_len);
    return &response;
}

/**
 * @brief Parse the request command
 * 
 * @param p_cmd Pointer to the command to parse
 * @param cmd_len The length of the command
 * @return app_cmd_resp_t* The response command of request command
 */
app_cmd_resp_t* app_cmd_parse_request(uint8_t* p_cmd, uint16_t cmd_len)
{
    NRF_LOG_HEXDUMP_INFO((uint8_t*)p_cmd, cmd_len);
    uint8_t   opcode              = p_cmd[0];
    uint8_t   req_payload_length  = p_cmd[1];
    uint8_t   req_payload_offset  = REQ_HEADER_LEN;

    uint8_t   resp_status         = STATUS_SUCCESS;
    uint8_t   resp_payload_length = 0;
    uint8_t*  resp_payload        = NULL;

    uint16_t cmd_crc16;
    memcpy((uint8_t*)&cmd_crc16, p_cmd + cmd_len - CRC_LEN, CRC_LEN);
    uint16_t cal_crc16 = crc16_compute(p_cmd, cmd_len - CRC_LEN, NULL);

    if (cmd_crc16 != cal_crc16)
    {
        resp_status = STATUS_CRC_ERR;
    }
    else if (cmd_len != (REQ_HEADER_LEN + req_payload_length + CRC_LEN))
    {
        return NULL;
    }
    else
    {
        switch(opcode)
        {
            case OPCODE_BLE_STAT_GET:
                if (req_payload_length == 0)
                {
                    resp_payload = app_ble_state_get();
                    resp_payload_length = 2;
                }
                else
                {
                    resp_status = STATUS_PAYLOAD_LEN_ERR;
                }
                break;

            case OPCODE_BLE_ADV_START:
                if (req_payload_length >= (BLE_GAP_PASSKEY_LEN + 1 + sizeof(uint32_t)*2 + APP_BLE_MA_SP_DATA_LEN_MIN) &&
                    req_payload_length <= (BLE_GAP_PASSKEY_LEN + 1 + sizeof(uint32_t)*2 + APP_BLE_MA_SP_DATA_LEN_MAX))
                {
                    uint8_t* p_state = app_ble_state_get();
                    if ((*p_state & APP_BLE_STATE_MASK_CONNECTED) != 0)
                    {
                        resp_status = STATUS_INVALID;
                        break;
                    }
                    uint8_t offset = req_payload_offset;
                    uint8_t* p_passkey = &p_cmd[offset];
                    offset += BLE_GAP_PASSKEY_LEN;

                    bool whitelist_enable = p_cmd[offset];
                    offset++;

                    uint32_t adv_timeout, passkey_timeout;
                    memcpy((uint8_t*)&adv_timeout, &p_cmd[offset], sizeof(uint32_t));
                    offset += sizeof(uint32_t);

                    memcpy((uint8_t*)&passkey_timeout, &p_cmd[offset], sizeof(uint32_t));
                    offset += sizeof(uint32_t);

                    uint8_t* p_ma_sp_data = &p_cmd[offset];
                    uint8_t data_len = req_payload_length - BLE_GAP_PASSKEY_LEN - 1 - (sizeof(uint32_t)*2);

                    app_ble_start_adv(p_passkey, whitelist_enable, adv_timeout, passkey_timeout, p_ma_sp_data, data_len);
                }
                else
                {
                    resp_status = STATUS_PAYLOAD_LEN_ERR;
                }
                break;

            case OPCODE_BLE_ADV_STOP:
                if (req_payload_length == 0)
                {
                    uint8_t* p_state = app_ble_state_get();
                    if ((*p_state & APP_BLE_STATE_MASK_ADV) != 0)
                    {
                        app_ble_stop_adv();
                    }
                }
                else
                {
                    resp_status = STATUS_PAYLOAD_LEN_ERR;
                }
                break;

            case OPCODE_BLE_DISCONNECT:
                if (req_payload_length == 0)
                {
                    uint8_t* p_state = app_ble_state_get();
                    if ((*p_state & APP_BLE_STATE_MASK_CONNECTED) != 0)
                    {
                        app_ble_disconnect();
                    }
                }
                else
                {
                    resp_status = STATUS_PAYLOAD_LEN_ERR;
                }
                break;

            case OPCODE_BLE_WL_ADD:
                if (req_payload_length == BLE_GAP_ADDR_LEN + 1)
                {
                    uint8_t addr_type = p_cmd[req_payload_offset];
                    uint8_t* p_addr = &p_cmd[req_payload_offset + 1];
                    uint8_t addr[BLE_GAP_ADDR_LEN];
                    memcpy(addr, p_addr, BLE_GAP_ADDR_LEN);

                    if ((addr_type != BLE_GAP_ADDR_TYPE_PUBLIC) &&
                        (addr_type != BLE_GAP_ADDR_TYPE_RANDOM_STATIC))
                    {
                        resp_status = STATUS_INVALID;
                    }
                    else if ((addr_type == BLE_GAP_ADDR_TYPE_RANDOM_STATIC) && 
                             (addr[BLE_GAP_ADDR_LEN-1] < APP_BLE_ADDR_TYPE_RANDOM_STATIC_MIN))
                    {
                        resp_status = STATUS_INVALID;
                    }
                    else
                    {
                        app_ble_whitelist_add(addr, addr_type);
                    }
                }
                else
                {
                    resp_status = STATUS_PAYLOAD_LEN_ERR;
                }
                break;

            case OPCODE_BLE_DEL_PEERS:
                if (req_payload_length == 0)
                {
                    app_ble_peers_del();
                }
                else
                {
                    resp_status = STATUS_PAYLOAD_LEN_ERR;
                }
                break;

            default:
                resp_status = STATUS_OPCODE_ERR;
                break;
        }
    }
    return generate_response(opcode, resp_status, resp_payload, resp_payload_length);
}
