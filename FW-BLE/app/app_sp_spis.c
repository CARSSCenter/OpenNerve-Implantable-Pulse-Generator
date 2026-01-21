/**
 * @file app_sp_spis.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of SPI slave port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_sp_spis.h"
#include "app_sp.h"

#include "custom_board.h"
#include "nrf_drv_spis.h"
#include "nrf_gpio.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME APP_SP_SPIS
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

#define SPIS_INSTANCE 0 /**< SPIS instance index. */
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */

#define RX_BUF_SIZE                DATA_BUFFER_MAX_SIZE
#define TX_BUF_SIZE                (DATA_BUFFER_MAX_SIZE + 1)    //1 byte data length + data

static volatile uint8_t m_tx_buf[TX_BUF_SIZE];
static volatile uint8_t m_rx_buf[RX_BUF_SIZE];

static uint8_t tx_offset = 0;

static volatile bool spis_xfer_done; /**< Flag used to indicate that SPIS instance completed the transfer. */
static bool init = false;

/**
 * @brief SPIS user event handler.
 *
 * @param event SPI slave driver event
 */
static void spis_event_handler(nrf_drv_spis_event_t event)
{
    nrfx_spis_evt_type_t evt_type = event.evt_type;
    switch(evt_type)
    {
        case NRFX_SPIS_BUFFERS_SET_DONE:
            NRF_LOG_DEBUG("NRFX_SPIS_BUFFERS_SET_DONE");
            nrf_gpio_pin_set(BLE_RDY_PIN);
            init = true;
            spis_xfer_done = false;
            app_sp_on_tx_ready(SP_SPI);
            if (tx_offset > 0)
            {
                nrf_gpio_pin_set(BLE_REQ_PIN);
                NRF_LOG_INFO("BLE_REQ_PIN[H]");
                NRF_LOG_FLUSH();
            }
            break;

        case NRF_DRV_SPIS_XFER_DONE:
            NRF_LOG_DEBUG("NRF_DRV_SPIS_XFER_DONE");
            nrf_gpio_pin_clear(BLE_RDY_PIN);

            spis_xfer_done = true;
            size_t tx_size = event.tx_amount;
            size_t rx_size = event.rx_amount;
            if (nrf_gpio_pin_out_read(BLE_REQ_PIN) == 0 && rx_size > 0)
            {
              NRF_LOG_INFO("Rx(%d)[0x%02x]", rx_size, m_rx_buf[0]);
              NRF_LOG_HEXDUMP_INFO((uint8_t*)m_rx_buf, rx_size);
              NRF_LOG_FLUSH();
              app_sp_on_rx_data(SP_SPI, (uint8_t*)m_rx_buf, rx_size);
              memset((uint8_t*)m_rx_buf, 0, rx_size);
              memset((uint8_t*)m_tx_buf, 0, tx_size);
            }
            else if (nrf_gpio_pin_out_read(BLE_REQ_PIN) > 0 && tx_size > 1)
            {
              NRF_LOG_INFO("Tx(%d)[0x%02x]", tx_size-1, m_tx_buf[1]);
              NRF_LOG_HEXDUMP_INFO((uint8_t*)m_tx_buf+1, tx_size-1);
              NRF_LOG_FLUSH();
              nrf_gpio_pin_clear(BLE_REQ_PIN);
              NRF_LOG_INFO("BLE_REQ_PIN[L]");
              NRF_LOG_FLUSH();
              tx_offset -= (tx_size-1);
              for(int i = (int)tx_size;i < TX_BUF_SIZE;i++)
                m_tx_buf[i-tx_size+1] = m_tx_buf[i];
             
              m_tx_buf[0] = tx_offset;
              //NRF_LOG_INFO("tx_offset: %d", tx_offset);
            }
            APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, 
                                                    (uint8_t*)m_tx_buf, TX_BUF_SIZE, 
                                                    (uint8_t*)m_rx_buf, RX_BUF_SIZE));
            break;

        case NRFX_SPIS_EVT_TYPE_MAX:
            NRF_LOG_DEBUG("NRFX_SPIS_EVT_TYPE_MAX");
            break;
    }
    NRF_LOG_FLUSH();
}

/**
 * @brief Function for initializing the SPI slave.
 * 
 */
void app_sp_spis_init(void)
{
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG;
    spis_config.csn_pin               = BLE_CSn_PIN;
    spis_config.miso_pin              = BLE_MISO_PIN;
    spis_config.mosi_pin              = BLE_MOSI_PIN;
    spis_config.sck_pin               = BLE_SCK_PIN;

    nrf_gpio_cfg_output(BLE_RDY_PIN);
    nrf_gpio_cfg_output(BLE_REQ_PIN);
    nrf_gpio_pin_clear(BLE_RDY_PIN);
    nrf_gpio_pin_clear(BLE_REQ_PIN);

    memset((uint8_t*)m_tx_buf, 0, TX_BUF_SIZE);
    memset((uint8_t*)m_rx_buf, 0, RX_BUF_SIZE);
    tx_offset = 0;

    nrf_gpio_cfg_input(BLE_CSn_PIN, NRF_GPIO_PIN_PULLUP);
    while(nrf_gpio_pin_read(BLE_CSn_PIN) == 0)
    {
        __NOP();
    }

    init = false;
    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
    APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis, 
                                            (uint8_t*)m_tx_buf, TX_BUF_SIZE, 
                                            (uint8_t*)m_rx_buf, RX_BUF_SIZE));
    while(!init)
    {
        __NOP();
    }
}

/**
 * @brief Put byte to the buffer of the spis port.
 * 
 * @param byte The byte to put.
 * @return uint32_t Error Codes number
 */
static uint32_t app_sp_spis_put_byte(uint8_t byte)
{
    if (tx_offset == TX_BUF_SIZE-1)
        return NRF_ERROR_BUSY;

    m_tx_buf[tx_offset + 1] = byte;
    tx_offset++;
    m_tx_buf[0] = tx_offset;

    return NRF_SUCCESS;
}

/**
 * @brief Put data to the buffer of the spis port.
 * 
 * @param data The data to put.
 */
void app_sp_spis_put(uint8_t* data, uint16_t size)
{
        uint32_t err_code;
        for (uint16_t i = 0; i < size; i++)
        {
            do
            {
                while(spis_xfer_done);
                err_code = app_sp_spis_put_byte(data[i]);
                if (err_code == NRF_ERROR_BUSY)
                {
                    if (nrf_gpio_pin_out_read(BLE_RDY_PIN) == 1)
                    {
                        nrf_gpio_pin_set(BLE_REQ_PIN);
                        NRF_LOG_INFO("BLE_REQ_PIN[H]");
                        NRF_LOG_FLUSH();
                    }
                }
                else if (err_code != NRF_SUCCESS)
                {
                    NRF_LOG_ERROR("Failed receiving message. Error 0x%x. ", err_code);
                    APP_ERROR_CHECK(err_code);
                }
            } 
            while (err_code == NRF_ERROR_BUSY);
        }

        if (nrf_gpio_pin_out_read(BLE_RDY_PIN) == 1)
        {
          nrf_gpio_pin_set(BLE_REQ_PIN);
          NRF_LOG_INFO("BLE_REQ_PIN[H]");
          NRF_LOG_FLUSH();
        }
}