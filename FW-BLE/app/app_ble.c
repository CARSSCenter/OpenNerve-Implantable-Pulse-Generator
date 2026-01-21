/**
 * @file app_ble.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief All Bluetooth applications
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_ble.h"
#include "app_ble_nus.h"
#include "app_ble_pm.h"
#include "app_sp.h"

#include "nrf_sdh.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_gatt.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "custom_board.h"

#define NRF_LOG_MODULE_NAME APP_BLE
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"


#define DEVICE_NAME                         APP_DEVICE_NAME                             /**< Name of device. Will be included in the advertising data. */

#define APP_BLE_OBSERVER_PRIO               3                                           /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(7.5, UNIT_1_25_MS)             /**< Minimum acceptable connection interval, Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Maximum acceptable connection interval, Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                       0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY_MS   5000                                        /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY_MS    30000                                       /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_BLE_PACKET_DATA_LEN             (BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)

NRF_BLE_GATT_DEF(m_gatt);                                                           /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                             /**< Context for the Queued Write module.*/
APP_TIMER_DEF(sec_check_tmr);
app_timer_t* p_sec_check_tmr = (app_timer_t*)sec_check_tmr;

static uint16_t   m_conn_handle          = BLE_CONN_HANDLE_INVALID;                 /**< Handle of the current connection. */
static uint16_t   m_ble_nus_max_data_len = APP_BLE_PACKET_DATA_LEN;                 /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

uint16_t* p_conn_handle = &m_conn_handle;

typedef struct
{
    uint8_t status;
    uint8_t disconnection_reason;
} ble_status_t;

static ble_status_t ble_status = {
    .status = 0xFF,
    .disconnection_reason = 0,
};

static uint32_t sec_check_timeout = 0;

/**
 * @brief Function for handling BLE events
 * 
 * @param p_ble_evt Bluetooth stack event
 * @param p_context Unused
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    UNUSED_PARAMETER(p_context);
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_DEBUG("BLE_GAP_EVT_CONNECTED");
            NRF_LOG_INFO("Connected...");
            ble_status.disconnection_reason = 0;
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            if (sec_check_timeout > 0)
            {
                err_code = app_timer_start(sec_check_tmr, APP_TIMER_TICKS(sec_check_timeout*1000), NULL);
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_DEBUG("BLE_GAP_EVT_DISCONNECTED");
            NRF_LOG_INFO("Disconnected...");
            ble_status.disconnection_reason = p_ble_evt->evt.gap_evt.params.disconnected.reason;

            err_code = app_timer_stop(sec_check_tmr);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("BLE_GAP_EVT_PHY_UPDATE_REQUEST");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } 
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            NRF_LOG_DEBUG("BLE_GATTS_EVT_SYS_ATTR_MISSING");
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            NRF_LOG_DEBUG("BLE_GATTC_EVT_TIMEOUT");
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            NRF_LOG_DEBUG("BLE_GATTS_EVT_TIMEOUT");
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            NRF_LOG_DEBUG("ble_evt_id: %d", p_ble_evt->header.evt_id);
            break;
    }
    NRF_LOG_FLUSH();
}

/**
 * @brief Function for the SoftDevice initialization. 
 * 
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**
 * @brief Function for the GAP initialization.
 * 
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    APP_ERROR_CHECK(sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) DEVICE_NAME, strlen(DEVICE_NAME)));

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for handling events from the GATT library.
 * 
 * @param p_gatt Contains status information for the GATT module.
 * @param p_evt GATT module event.
 */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Data len is set to 0x%X(%d)", m_ble_nus_max_data_len, m_ble_nus_max_data_len);
    }
    NRF_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}

/**
 * @brief Function for initializing the GATT library.
 * 
 */
static void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for handling an event from the Connection Parameters Module.
 * 
 * @param p_evt Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
        NRF_LOG_DEBUG("BLE_CONN_PARAMS_EVT_SUCCEEDED");
    }
    else if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        NRF_LOG_DEBUG("BLE_CONN_PARAMS_EVT_SUCCEEDED");
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
    NRF_LOG_FLUSH();
}

/**
 * @brief Function for handling errors from the Connection Parameters module.
 * 
 * @param nrf_error Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_CHECK(nrf_error);
}

/**
 * @brief Function for initializing the Connection Parameters module.
 * 
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(FIRST_CONN_PARAMS_UPDATE_DELAY_MS);
    cp_init.next_conn_params_update_delay  = APP_TIMER_TICKS(NEXT_CONN_PARAMS_UPDATE_DELAY_MS);
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);

    ble_conn_state_init();
}

/**
 * @brief Timer handler for Security check.
 * 
 * @param p_context Unused
 */
static void sec_check_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    ble_gap_conn_sec_t conn_sec = {0};
    APP_ERROR_CHECK(sd_ble_gap_conn_sec_get(*p_conn_handle, &conn_sec));

    NRF_LOG_INFO("Security Check.....");
    ble_gap_conn_sec_mode_t sec_check;
    BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(&sec_check);
    if (conn_sec.sec_mode.sm == sec_check.sm && conn_sec.sec_mode.lv == sec_check.lv)
    {
        NRF_LOG_INFO("Security mode & level: Pass");
        NRF_LOG_FLUSH();
    }
    else
    {
        NRF_LOG_INFO("Security mode & level: Fail, mode %d, level %d", conn_sec.sec_mode.sm, conn_sec.sec_mode.lv);
        NRF_LOG_INFO("Disconnect...");
        NRF_LOG_FLUSH();
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    }
}

/**
 * @brief Initialization of the Bluetooth application.
 * 
 */
void app_ble_init(void)
{
    ble_stack_init();
    gap_params_init();
    gatt_init();
    conn_params_init();

    app_ble_pm_init();
    app_ble_nus_init(&m_qwr);
    app_ble_adv_init();

    APP_ERROR_CHECK(app_timer_create(&sec_check_tmr, APP_TIMER_MODE_SINGLE_SHOT, sec_check_timer_handler));
}

/**
 * @brief Disconnect from Bluetooth application.
 * 
 */
void app_ble_disconnect(void)
{
    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
        APP_ERROR_CHECK(sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
}

/**
 * @brief Start advertising from bluetooth application.
 * 
 * @param passkey Pointer to 6-digit ASCII string (digit 0..9 only, no NULL termination) passkey to be used during pairing.
 * @param whitelist_enable Enable / disable whitelist
 * @param adv_timeout Timeout of advertising
 * @param passkey_timeout Timeout for entering passkey 
 * @param p_ma_sp_data Manufacturer specific data of advertising data
 * @param data_len Length of manufacturer specific data
 */
void app_ble_start_adv(uint8_t* passkey, bool whitelist_enable, uint32_t adv_timeout, uint32_t passkey_timeout, uint8_t* p_ma_sp_data, uint8_t data_len)
{
    app_ble_pm_passkey_set(passkey);
    app_ble_adv_start(whitelist_enable, adv_timeout, p_ma_sp_data, data_len);
    sec_check_timeout = passkey_timeout;
}

/**
 * @brief Stop advertising from bluetooth application.
 * 
 */
void app_ble_stop_adv(void)
{
    app_ble_adv_stop();
}

/**
 * @brief Add an address to the whitelist
 * 
 * @param addr Pointer to 48-bit address, LSB format.
 * @param addr_type Address types, see @ref BLE_GAP_ADDR_TYPES. Only BLE_GAP_ADDR_TYPE_PUBLIC or BLE_GAP_ADDR_TYPE_RANDOM_STATIC is valid.
 */
void app_ble_whitelist_add(uint8_t* addr, uint8_t addr_type)
{
    app_ble_pm_whitelist_add(addr, addr_type);
}

/**
 * @brief Function for deleting all data stored for all peers.
 *
 */
void app_ble_peers_del(void)
{
    app_ble_pm_peers_del();
}

/**
 * @brief Send data from bluetooth application.
 * 
 * @param p_data The data to send.
 * @param p_length The length of the data to send.
 * @return uint32_t Error Codes number
 */
uint32_t app_ble_data_send(uint8_t* p_data, uint16_t* p_length)
{
    return app_ble_nus_data_send(p_data, p_length);
}

/**
 * @brief Get the status of BLE
 * 
 * @return uint8_t* The status of BLE
 */
uint8_t* app_ble_state_get(void)
{
    return (uint8_t*)&ble_status;
}

/**
 * @brief Handler for updating BLE status.
 * 
 */
void app_ble_state_handler(void)
{
    bool adv_enable = app_ble_adv_state_get();
    bool connected = (m_conn_handle != BLE_CONN_HANDLE_INVALID)?true:false;
    bool sec_enable = app_ble_pm_sec_state_get() & connected;
    bool nus_ready = app_ble_nus_is_ready() & connected;
    uint8_t new_ble_status =  nus_ready*APP_BLE_STATE_MASK_NUS_READY + 
                              sec_enable*APP_BLE_STATE_MASK_SECURITY + 
                              connected*APP_BLE_STATE_MASK_CONNECTED + 
                              adv_enable*APP_BLE_STATE_MASK_ADV;
    
    if (ble_status.status != new_ble_status)
    {
        ble_status.status = new_ble_status;
        NRF_LOG_INFO("Status Changed(0x%02x, 0x%02x)", ble_status.status, ble_status.disconnection_reason);
        NRF_LOG_FLUSH();
    }
}