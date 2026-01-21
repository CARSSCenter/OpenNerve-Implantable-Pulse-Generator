/**
 * @file app_ble_adv.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of Bluetooth advertising
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_ble_adv.h"

#include "ble_nus.h"
#include "app_error.h"
#include "peer_manager.h"

#define NRF_LOG_MODULE_NAME APP_BLE_ADV
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

/**
 * @brief Company Identifier
 * @note This value(0xFFFF) may be used in the internal and interoperability tests before a Company ID has been assigned.
 * @note This value(0xFFFF) shall not be used in shipping end products.
 */
#define COMPANY_ID                          0xFFFF

#define NUS_SERVICE_UUID_TYPE               BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */
#define FAST_ADV_INTERVAL                   64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define FAST_ADV_DURATION                   1000                                        /**< The advertising duration (10 seconds) in units of 10 milliseconds. */
#define SLOW_ADV_INTERVAL                   160                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 100 ms). */
#define SLOW_ADV_DURATION                   59000                                       /**< The advertising duration (590 seconds) in units of 10 milliseconds. */

BLE_ADVERTISING_DEF(m_advertising);                                                 /**< Advertising module instance. */

ble_advertising_t* p_adv = &m_advertising;

static ble_advertising_init_t adv_init;
static ble_uuid_t m_adv_uuids[]          =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}
};

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;
static uint8_t manuf_data_data[MAX_ADV_MANUF_DATA_DATA_LEN] = {0};
static ble_advdata_manuf_data_t manuf_data = {
    COMPANY_ID, 
    {MAX_ADV_MANUF_DATA_DATA_LEN, (uint8_t*)manuf_data_data}
};

static bool adv_enable = false;

 /**
  * @brief Function for handling advertising events.
  * 
  * @param ble_adv_evt 
  */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST_WHITELIST:
            NRF_LOG_INFO("[Whitelist]");
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("ADV enabled...");
            uint32_t timeout = (m_advertising.adv_modes_config.ble_adv_fast_timeout + 
                                m_advertising.adv_modes_config.ble_adv_slow_timeout) / 100;
            NRF_LOG_INFO("ADV timeout = %d sec", timeout);
            m_advertising.adv_modes_config.ble_adv_on_disconnect_disabled = true;
            adv_enable = true;
            break;

        case BLE_ADV_EVT_SLOW_WHITELIST:
        case BLE_ADV_EVT_SLOW:
            adv_enable = true;
            break;

        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("ADV disabled...");
            adv_enable = false;
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST:
            NRF_LOG_DEBUG("BLE_ADV_EVT_WHITELIST_REQUEST");
            ble_gap_addr_t addrs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t irks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            uint32_t addr_cnt, irk_cnt;

            addr_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            irk_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            APP_ERROR_CHECK(pm_whitelist_get(addrs, &addr_cnt, irks, &irk_cnt));
            APP_ERROR_CHECK(ble_advertising_whitelist_reply(&m_advertising, addrs, addr_cnt, irks, irk_cnt));
            /*
            for(int i=0;i<addr_cnt;i++)
            {
                NRF_LOG_INFO("Whitelist address(%d):", i+1);
                NRF_LOG_HEXDUMP_INFO(addrs[i].addr,BLE_GAP_ADDR_LEN);
            }
            for(int i=0;i<irk_cnt;i++)
            {
                NRF_LOG_INFO("Whitelist irk(%d):", i+1);
                NRF_LOG_HEXDUMP_INFO(irks[i].irk,BLE_GAP_SEC_KEY_LEN);
            }
            */
            break;

        default:
            break;
    }
}

/**
 * @brief Function for initializing the Advertising functionality.
 * 
 */
void app_ble_adv_init(void)
{
    memset(&adv_init, 0, sizeof(adv_init));

    adv_init.advdata.include_appearance = false;
    adv_init.advdata.flags              = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    adv_init.advdata.p_manuf_specific_data = &manuf_data;

    adv_init.srdata.name_type          = BLE_ADVDATA_FULL_NAME;
    adv_init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    adv_init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    adv_init.config.ble_adv_whitelist_enabled = true;
    adv_init.config.ble_adv_on_disconnect_disabled = true;

    adv_init.config.ble_adv_fast_enabled  = true;
    adv_init.config.ble_adv_fast_interval = FAST_ADV_INTERVAL;
    adv_init.config.ble_adv_fast_timeout  = FAST_ADV_DURATION;
    adv_init.config.ble_adv_slow_enabled  = true;
    adv_init.config.ble_adv_slow_interval = SLOW_ADV_INTERVAL;
    adv_init.config.ble_adv_slow_timeout  = SLOW_ADV_DURATION;

    adv_init.evt_handler = on_adv_evt;

    APP_ERROR_CHECK(ble_advertising_init(&m_advertising, &adv_init));

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**
 * @brief Function for start the Advertising functionality.
 * 
 * @param whitelist_enable Enable / disable whitelist
 * @param adv_timeout Timeout of advertising
 * @param p_ma_sp_data Manufacturer specific data of advertising data
 * @param data_len Length of manufacturer specific data
 */
void app_ble_adv_start(bool whitelist_enable, uint32_t adv_timeout, uint8_t* p_ma_sp_data, uint8_t data_len)
{
    if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        uint32_t adv_timeout_10ms = adv_timeout * 100;   //In units of 10ms
        if (adv_timeout_10ms <= FAST_ADV_DURATION)
        {
            m_advertising.adv_modes_config.ble_adv_fast_timeout = adv_timeout_10ms;
            m_advertising.adv_modes_config.ble_adv_slow_timeout = 0;
            m_advertising.adv_modes_config.ble_adv_slow_enabled  = false;
        }
        else
        {
            m_advertising.adv_modes_config.ble_adv_fast_timeout = FAST_ADV_DURATION;
            m_advertising.adv_modes_config.ble_adv_slow_timeout = adv_timeout_10ms - FAST_ADV_DURATION;
            m_advertising.adv_modes_config.ble_adv_slow_enabled  = true;
        }

        ble_advdata_t const * const p_advdata = &adv_init.advdata;
        ble_advdata_t const * const p_srdata = &adv_init.srdata;
        p_advdata->p_manuf_specific_data->company_identifier = *((uint16_t*)p_ma_sp_data);
        uint8_t ma_sp_data_data_len = data_len - AD_TYPE_MANUF_SPEC_DATA_ID_SIZE;
        p_advdata->p_manuf_specific_data->data.size = ma_sp_data_data_len;
        memcpy(p_advdata->p_manuf_specific_data->data.p_data, &p_ma_sp_data[AD_TYPE_MANUF_SPEC_DATA_ID_SIZE], ma_sp_data_data_len);

        if (adv_enable)
            sd_ble_gap_adv_stop(m_advertising.adv_handle);

        APP_ERROR_CHECK(ble_advertising_advdata_update(&m_advertising, p_advdata, p_srdata));
        APP_ERROR_CHECK(ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST));
        if (!whitelist_enable)
            APP_ERROR_CHECK(ble_advertising_restart_without_whitelist(&m_advertising));
    }
}

/**
 * @brief Function for stop the Advertising functionality.
 * 
 */
void app_ble_adv_stop(void)
{
    if (!adv_enable)
        return;

    sd_ble_gap_adv_stop(m_advertising.adv_handle);
    APP_ERROR_CHECK(ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE));
}

/**
 * @brief Get the status of the advertising
 * 
 * @return true Advertising starts
 * @return false Advertising stops
 */
bool app_ble_adv_state_get(void)
{
    return adv_enable;
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
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            app_ble_adv_stop();
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        default:

            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_observer, BLE_ADV_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);