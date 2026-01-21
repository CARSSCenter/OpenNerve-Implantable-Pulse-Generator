/**
 * @file test_app_cmd.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Test the applications of command
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "test_app_cmd.h"

#include "app_cmd.h"
#include "unity.h"
#include "crc16.h"
#include "nrf_delay.h"

/**
 * @brief Test request command parser and response command generator application
 * 
 */
static void test_app_cmd_req_resp (void)
{
    uint8_t cmd_req[] = {0x50, 0x00, 0xB0, 0x13};
    app_cmd_resp_t* p_resp = app_cmd_parse_request((uint8_t*)cmd_req, sizeof(cmd_req));

    uint16_t exp_crc16 = crc16_compute(p_resp->cmd_resp, p_resp->cmd_resp_len - CRC_LEN, NULL);
    uint16_t act_crc16 = *((uint16_t*)&p_resp->cmd_resp[p_resp->cmd_resp_len - CRC_LEN]);
    uint8_t exp_len = p_resp->cmd_resp_len;
    uint8_t act_len = p_resp->cmd_resp[1] + RESP_HEADER_LEN + CRC_LEN;
    uint8_t exp_status = STATUS_SUCCESS;
    uint8_t act_status = p_resp->cmd_resp[2];

    TEST_ASSERT_EQUAL_UINT16(exp_crc16, act_crc16);
    TEST_ASSERT_EQUAL_UINT8(exp_len, act_len);
    TEST_ASSERT_EQUAL_UINT8(exp_status, act_status);
    nrf_delay_ms(500);
}

/**
 * @brief Run all test items for command applications
 * 
 */
void test_run_app_cmd (void)
{
    UnityPrint("***********************");
    UNITY_PRINT_EOL();

    UnityBegin("[CMD]");
    RUN_TEST(test_app_cmd_req_resp, __LINE__);
    UNITY_END();
    nrf_delay_ms(500);
}