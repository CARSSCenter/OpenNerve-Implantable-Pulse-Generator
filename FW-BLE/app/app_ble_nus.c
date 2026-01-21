/**
 * @file app_ble_nus.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of Bluetooth Nordic UART Service
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_ble_nus.h"

#include "app_sp.h"

#define NRF_LOG_MODULE_NAME APP_BLE_NUS
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */
ble_nus_t* p_nus = &m_nus;

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

static bool sec_enable = false;

/**
 * @brief Function for handling Queued Write Module errors.
 * 
 * @param nrf_error Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_CHECK(nrf_error);
}

/**
 * @brief Function for handling the data from the Nordic UART Service.
 * 
 * @param p_evt Nordic UART Service event.
 */
static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    m_conn_handle = p_evt->conn_handle;

    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        NRF_LOG_INFO("Rx(%d)[0x%02x]", p_evt->params.rx_data.length, p_evt->params.rx_data.p_data[0]);

        if (sec_enable)
        {
            app_sp_put(SP_ALL, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        }
        else
        {
            APP_ERROR_CHECK(NRF_ERROR_INVALID_STATE);
        }
    }
    else if (p_evt->type == BLE_NUS_EVT_TX_RDY)
    {
        NRF_LOG_DEBUG("BLE_NUS_EVT_TX_RDY");
    }
    else if (p_evt->type == BLE_NUS_EVT_COMM_STARTED)
    {
        NRF_LOG_DEBUG("BLE_NUS_EVT_COMM_STARTED");
    }
    else if (p_evt->type == BLE_NUS_EVT_COMM_STOPPED)
    {
        NRF_LOG_DEBUG("BLE_NUS_EVT_COMM_STOPPED");
    }
    NRF_LOG_FLUSH();
}

/**
 * @brief Function for initializing services that will be used by the application.
 * 
 * @param p_qwr Queued Writes structure. This structure must be
 *              supplied by the application. It is initialized by this function
 *              and is later used to identify the particular Queued Writes instance.
 */
void app_ble_nus_init(nrf_ble_qwr_t* p_qwr)
{
    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(p_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for sending a data to the peer.
 * 
 * @param p_data Data to be sent.
 * @param p_length Pointer Length of the data. Amount of sent bytes.
 * @return uint32_t If the data was sent successfully. Otherwise, an error code is returned.
 */
uint32_t app_ble_nus_data_send(uint8_t* p_data, uint16_t* p_length)
{
    if (sec_enable)
    {
        NRF_LOG_INFO("Tx(%d)[0x%02x]", *p_length, p_data[0]);
        return ble_nus_data_send(&m_nus, p_data, p_length, m_conn_handle);
    }
    else
    {
        return NRF_ERROR_INVALID_STATE;
    }
}

/**
 * @brief Checks if this service is ready to send.
 * 
 * @return true The service is ready to send.
 * @return false The service is not ready to send.
 */
bool app_ble_nus_is_ready(void)
{
    ble_nus_client_context_t    * p_client;
    bool is_ready = false;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        APP_ERROR_CHECK(blcm_link_ctx_get(p_nus->p_link_ctx_storage,
                                          m_conn_handle,
                                          (void *) &p_client));
        is_ready = p_client->is_notification_enabled;
    }

    return is_ready;
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
        case BLE_GAP_EVT_DISCONNECTED:
            sec_enable = false;
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_CONN_SEC_UPDATE:
        {
            uint8_t sec_mode = p_ble_evt->evt.gap_evt.params.conn_sec_update.conn_sec.sec_mode.sm;
            uint8_t sec_level = p_ble_evt->evt.gap_evt.params.conn_sec_update.conn_sec.sec_mode.lv;
            ble_gap_conn_sec_mode_t sec_check;
            BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(&sec_check);
            if (sec_mode == sec_check.sm && sec_level == sec_check.lv)
                sec_enable = true;
        }
            break;

        default:
            
            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_observer, BLE_NUS_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);