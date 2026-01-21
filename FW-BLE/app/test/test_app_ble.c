/**
 * @file test_app_ble.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Test the applications of Bluetooth
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "test_app_ble.h"

#include "app_ble.h"
#include "app_ble_adv.h"
#include "app_ble_nus.h"
#include "app_ble_pm.h"

#include "peer_manager.h"

#include <stdbool.h>
#include "custom_board.h"
#include "nrf_delay.h"

#define TEST_ADV_TIMEOUT      15
#define TEST_PASSKEY_TIMEOUT  20

static uint8_t exp_wl_address[BLE_GAP_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
static uint8_t exp_passkey[BLE_GAP_PASSKEY_LEN] = APP_BLE_PASSKEY;
static uint8_t exp_manuf_data[] = {0xAA, 0xBB, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x01, 0x02, 0x03, 0x04};

static uint8_t act_passkey[BLE_GAP_PASSKEY_LEN];

static bool is_connected = false;

/**
 * @brief Test the application that starts Bluetooth advertising on idle
 * 
 */
static void test_app_ble_start_adv_on_idle (void)
{
    pm_peers_delete();
    nrf_delay_ms(1000);
    exp_wl_address[BLE_GAP_ADDR_LEN-1] |= 0xC0;
    app_ble_whitelist_add(exp_wl_address, BLE_GAP_ADDR_TYPE_RANDOM_STATIC);
    nrf_delay_ms(1000);

    app_ble_start_adv(exp_passkey, true, TEST_ADV_TIMEOUT, TEST_PASSKEY_TIMEOUT, exp_manuf_data, sizeof(exp_manuf_data));

    for (uint32_t i=0;i<1000;i++)
    {
        nrf_delay_ms(1);
        app_ble_state_handler();
    }    

    uint8_t* p_act_state = app_ble_state_get();
    uint8_t act_mode = (uint8_t)p_adv->adv_mode_current;
    bool act_wl_enable = p_adv->whitelist_in_use;
    uint8_t* act_adv_data = p_adv->adv_data.adv_data.p_data;
    uint16_t act_adv_data_len = p_adv->adv_data.adv_data.len;
    uint16_t act_manuf_offset = 0;
    uint16_t act_manuf_data_len = ble_advdata_search(act_adv_data, act_adv_data_len, &act_manuf_offset, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);

    uint8_t exp_mode = BLE_ADV_MODE_FAST;
    uint8_t exp_state = 0b00000001;
    uint16_t exp_manuf_data_len = sizeof(exp_manuf_data);

    TEST_ASSERT_EQUAL_UINT8(exp_mode, act_mode);
    TEST_ASSERT_TRUE(act_wl_enable);
    TEST_ASSERT_EQUAL_UINT16(exp_manuf_data_len, act_manuf_data_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp_manuf_data, &act_adv_data[act_manuf_offset], exp_manuf_data_len);
    TEST_ASSERT_EQUAL_UINT8(exp_state, *p_act_state);
}

/**
 * @brief Test the application that starts Bluetooth advertising while connected
 * 
 */
static void test_app_ble_start_adv_on_conn (void)
{
    app_ble_start_adv(exp_passkey, false, TEST_ADV_TIMEOUT, TEST_PASSKEY_TIMEOUT, exp_manuf_data, sizeof(exp_manuf_data));
    uint8_t act_mode = (uint8_t)p_adv->adv_mode_current;

    uint8_t exp_mode = BLE_ADV_MODE_IDLE;
    TEST_ASSERT_EQUAL_UINT8(exp_mode, act_mode);
}

/**
 * @brief Test the application that stops Bluetooth advertising
 * 
 */
static void test_app_ble_stop_adv (void)
{
    app_ble_stop_adv();
    uint8_t act_mode = (uint8_t)p_adv->adv_mode_current;

    uint8_t exp_mode = BLE_ADV_MODE_IDLE;
    TEST_ASSERT_EQUAL_UINT8(exp_mode, act_mode);
}

/**
 * @brief Test passkey for the pairing application
 * 
 */
static void test_app_ble_adv_passkey (void)
{
    TEST_ASSERT_EQUAL_UINT8_ARRAY(exp_passkey, act_passkey, sizeof(exp_passkey));
}

/**
 * @brief Test security in the peer manager application
 * 
 */
static void test_app_ble_pm_sec_mode_level (void)
{
    ble_gap_conn_sec_t act_sec = {0};
    APP_ERROR_CHECK(sd_ble_gap_conn_sec_get(*p_conn_handle, &act_sec));

    ble_gap_conn_sec_mode_t exp_sec;
    BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(&exp_sec);

    TEST_ASSERT_EQUAL_UINT8(exp_sec.sm, act_sec.sec_mode.sm);
    TEST_ASSERT_EQUAL_UINT8(exp_sec.lv, act_sec.sec_mode.lv);
}

/**
 * @brief Test the ready confirmation in the NUS application
 * 
 */
static void test_app_ble_nus_is_ready (void)
{
    TEST_ASSERT_TRUE(app_ble_nus_is_ready());
}

/**
 * @brief Test data sending in NUS application
 * 
 */
static void test_app_ble_nus_data_send (void)
{
    uint8_t data[BLE_NUS_MAX_DATA_LEN];
    uint16_t length = sizeof(data);
    for(int i=0;i<length;i++)
        data[i] = i+1;

    uint32_t errcode = app_ble_nus_data_send(data, &length);
    TEST_ASSERT_EQUAL_UINT32(NRF_SUCCESS, errcode);
}

/**
 * @brief Test connection confirmation timer in Bluetooth application
 * 
 */
static void test_app_ble_conn_check_timer (void)
{
    TEST_ASSERT_NOT_EQUAL(APP_TIMER_IDLE_VAL, p_sec_check_tmr->end_val);
}

/**
 * @brief Test active disconnect in Bluetooth application
 * 
 */
static void test_app_ble_disconnect (void)
{
    app_ble_disconnect();
    for (uint32_t i=0;i<1000;i++)
    {
        nrf_delay_ms(1);
        app_ble_state_handler();
    }
    uint8_t* p_act_state = app_ble_state_get();
    uint8_t exp_state = 0b00000000;
    TEST_ASSERT_EQUAL_UINT8(exp_state, *p_act_state);
}

/**
 * @brief At the time of delay, execute the request handler of LESC at the same time
 * 
 * @param ms The time of delay
 */
static void delay_ms_with_lesc_req (uint32_t ms)
{
   for (uint32_t i=0;i<ms;i++)
   {
      nrf_delay_ms(1);
      APP_ERROR_CHECK(nrf_ble_lesc_request_handler());
   }    
}

/**
 * @brief Run all tests in Bluetooth idle state
 * 
 */
void test_run_app_ble_on_idle (void)
{
    UnityPrint("***********************");
    UNITY_PRINT_EOL();

    UnityBegin("[BLE_IDLE]");
    RUN_TEST(test_app_ble_start_adv_on_idle, __LINE__);
    RUN_TEST(test_app_ble_stop_adv, __LINE__);
    UNITY_END();
    nrf_delay_ms(500);
}

/**
 * @brief Run all the test items when the Bluetooth is connected
 * 
 */
void test_run_app_ble_on_connected (void)
{
    UnityPrint("***********************");
    UNITY_PRINT_EOL();

    app_ble_start_adv(exp_passkey, false, TEST_ADV_TIMEOUT, TEST_PASSKEY_TIMEOUT, exp_manuf_data, sizeof(exp_manuf_data));
    UnityPrint("...Waiting for connect");
    UNITY_PRINT_EOL();
    while(!is_connected)
        nrf_delay_ms(100);
    
    delay_ms_with_lesc_req(500);

    UnityBegin("[BLE_CONN]");
    RUN_TEST(test_app_ble_start_adv_on_conn, __LINE__);
    RUN_TEST(test_app_ble_conn_check_timer, __LINE__);

    delay_ms_with_lesc_req(TEST_PASSKEY_TIMEOUT*1000 + 500);

    RUN_TEST(test_app_ble_adv_passkey, __LINE__); 
    RUN_TEST(test_app_ble_pm_sec_mode_level, __LINE__);
    RUN_TEST(test_app_ble_nus_is_ready, __LINE__);
    RUN_TEST(test_app_ble_nus_data_send, __LINE__);
    UNITY_END();
    nrf_delay_ms(500);
}

/**
 * @brief Run all the test items when the Bluetooth is disconnected
 * 
 */
void test_run_app_ble_on_disconnected (void)
{
    UnityPrint("***********************");
    UNITY_PRINT_EOL();

    UnityBegin("[BLE_DISC]");
    RUN_TEST(test_app_ble_disconnect, __LINE__);
    UNITY_END();
    nrf_delay_ms(500);
}

/**
 * @brief Function for handling BLE events.
 *
 * @param   p_ble_evt       Event received from the BLE stack.
 * @param   p_context       Context.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    UNUSED_PARAMETER(p_context);

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            is_connected = true;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            is_connected = false;
            break;

        case BLE_GAP_EVT_PASSKEY_DISPLAY:
            memcpy(act_passkey, p_ble_evt->evt.gap_evt.params.passkey_display.passkey, sizeof(act_passkey));
            break;

        default:

            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_evt_observer, PM_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);