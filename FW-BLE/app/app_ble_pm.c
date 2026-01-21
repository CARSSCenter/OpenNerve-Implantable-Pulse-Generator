/**
 * @file app_ble_pm.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications for the peer manager
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_ble_pm.h"

#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "ble.h"
#include "nrf_ble_lesc.h"
#include "nrf_sdh_ble.h"
#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME APP_BLE_PM
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

static pm_conn_sec_status_t min_conn_sec;

static uint8_t app_passkey[BLE_GAP_PASSKEY_LEN] = APP_BLE_PASSKEY;

static uint16_t m_conn_handle;

static bool sec_enable = false;

static bool whitelist_reload = false;
static pm_peer_data_bonding_t wl_peer_data;

/**
 * @brief Function for loading data from peers to the whitelist
 * 
 */
static void load_peers_to_whitelist(void)
{
    pm_peer_id_t list[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
    uint32_t list_size = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    APP_ERROR_CHECK(pm_peer_id_list(list, &list_size, PM_PEER_ID_INVALID, PM_PEER_ID_LIST_ALL_ID));
    APP_ERROR_CHECK(pm_device_identities_list_set(list, list_size));
    APP_ERROR_CHECK(pm_whitelist_set(list, list_size));
    NRF_LOG_INFO("Load peers(%d) to whitelist", list_size);
}

/**
 * @brief Function for handling Peer Manager events.
 * 
 * @param p_evt Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_disconnect_on_sec_failure(p_evt);
    pm_handler_flash_clean(p_evt);
    pm_handler_disconnect_on_insufficient_sec(p_evt, &min_conn_sec);

    pm_conn_sec_config_t    peer_conn_sec_config = { true };
    m_conn_handle = p_evt->conn_handle;

    switch (p_evt->evt_id)
    {
        case PM_EVT_CONN_SEC_CONFIG_REQ:
            pm_conn_sec_config_reply(m_conn_handle, &peer_conn_sec_config);
            break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
            sec_enable = true;
            break;

        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
            if(p_evt->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_PEER_RANK)
            {
                if (p_evt->params.peer_data_update_succeeded.flash_changed) 
                {
                    if (pm_peer_count() > BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
                    {
                        pm_peer_id_t lowest_ranked_peer;
                        APP_ERROR_CHECK(pm_peer_ranks_get(NULL, NULL, &lowest_ranked_peer, NULL));
                        APP_ERROR_CHECK(pm_peer_delete(lowest_ranked_peer));
                        NRF_LOG_INFO("Delete lowest ranked peer");
                    }
                    else
                    {
                        load_peers_to_whitelist();
                    }
                }   
            }
            else if(p_evt->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_BONDING)
            {
                if (whitelist_reload)
                {
                    APP_ERROR_CHECK(pm_peer_rank_highest(p_evt->peer_id));
                    whitelist_reload = false;
                }
            }
            break;

        case PM_EVT_PEER_DELETE_SUCCEEDED:
            load_peers_to_whitelist();
            break;

        default:
            //NRF_LOG_INFO("pm_evt_id: %d", p_evt->evt_id);
            break;
    }
    NRF_LOG_FLUSH();
}

/**
 * @brief Function for initializing the Peer Manager.
 * 
 */
void app_ble_pm_init(void)
{
    ble_gap_sec_params_t sec_param;
    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));
    memset(&min_conn_sec, 0, sizeof(pm_conn_sec_status_t));
    APP_ERROR_CHECK(pm_init());

    // Security parameters to be used for all security procedures.
    sec_param.bond            = true;
    sec_param.mitm            = true;
    sec_param.lesc            = PM_LESC_ENABLED;
    sec_param.keypress        = false;
    sec_param.io_caps         = BLE_GAP_IO_CAPS_DISPLAY_ONLY;
    sec_param.oob             = false;

    sec_param.min_key_size    = 7;
    sec_param.max_key_size    = 16;
    sec_param.kdist_own.enc   = 1;
    sec_param.kdist_own.id    = 1;
    sec_param.kdist_peer.enc  = 1;
    sec_param.kdist_peer.id   = 1;

    min_conn_sec.connected      = 1;
    min_conn_sec.encrypted      = 1;
    min_conn_sec.mitm_protected = 1;
    min_conn_sec.bonded         = 1;
    min_conn_sec.lesc           = 1;
    min_conn_sec.reserved       = 3;

    APP_ERROR_CHECK(pm_sec_params_set(&sec_param));
    APP_ERROR_CHECK(pm_register(pm_evt_handler));

    app_ble_pm_passkey_set(app_passkey);
    load_peers_to_whitelist();
}

/**
 * @brief Set the passkey to be used during pairing.
 * 
 * @param passkey Pointer to 6-digit ASCII string (digit 0..9 only, no NULL termination) passkey to be used during pairing.
 */
void app_ble_pm_passkey_set(uint8_t* passkey)
{
    ble_opt_t opt;
    memcpy(app_passkey, passkey, BLE_GAP_PASSKEY_LEN);
    opt.gap_opt.passkey.p_passkey = (uint8_t*)app_passkey;
    APP_ERROR_CHECK(sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &opt));
}

/**
 * @brief Add an address to the whitelist
 * 
 * @param addr Pointer to 48-bit address, LSB format.
 * @param addr_type Address types, see @ref BLE_GAP_ADDR_TYPES. Only BLE_GAP_ADDR_TYPE_PUBLIC or BLE_GAP_ADDR_TYPE_RANDOM_STATIC is valid.
 */
void app_ble_pm_whitelist_add(uint8_t* addr, uint8_t addr_type)
{
    pm_peer_id_t peer_id;
    memset(&wl_peer_data, 0, sizeof(pm_peer_data_bonding_t));

    wl_peer_data.own_role = BLE_GAP_ROLE_PERIPH;
    wl_peer_data.peer_ble_id.id_addr_info.addr_type = addr_type;
    memcpy(wl_peer_data.peer_ble_id.id_addr_info.addr, addr, BLE_GAP_ADDR_LEN);

    whitelist_reload = true;
    APP_ERROR_CHECK(pm_peer_new(&peer_id, &wl_peer_data, NULL));
}

/**
 * @brief Function for deleting all data stored for all peers.
 *
 */
void app_ble_pm_peers_del(void)
{
    APP_ERROR_CHECK(pm_peers_delete());
}

/**
 * @brief Get the status of security.
 * 
 * @return true Security is enabled.
 * @return false Security is not enabled.
 */
bool app_ble_pm_sec_state_get(void)
{
    return sec_enable;
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

    m_conn_handle = p_ble_evt->evt.common_evt.conn_handle;
    pm_handler_secure_on_connection(p_ble_evt);

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_PASSKEY_DISPLAY:
            NRF_LOG_INFO("Passkey: %s", p_ble_evt->evt.gap_evt.params.passkey_display.passkey);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            sec_enable = false;
            break;

        default:
            //NRF_LOG_INFO("BLE evt id: %d", p_ble_evt->header.evt_id - BLE_GAP_EVT_BASE);
            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_evt_observer, PM_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);