/**
 * @file app_sp.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of serial port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_sp.h"

#include "app_ble.h"
#include "app_sp_uart.h"
#include "app_sp_spis.h"
#include "app_cmd.h"

#include "app_timer.h"



#include "custom_board.h"

#define NRF_LOG_MODULE_NAME APP_SP
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

static uint8_t data_buffer[DATA_BUFFER_GROUP_COUNT][DATA_BUFFER_MAX_SIZE];
static uint16_t data_lengths[DATA_BUFFER_GROUP_COUNT];

static uint8_t write_idx = 0;
static uint8_t read_idx = 0;
static uint8_t data_count = 0;

static bool init = false;

typedef struct
{
    app_cmd_resp_t* p_resp_cmd;
    uint8_t resp_port;
} app_sp_cmd_resp_t;

static app_sp_cmd_resp_t response;

static void buffer_add(const uint8_t* data, uint16_t size) {
    if (size > DATA_BUFFER_MAX_SIZE) {
        size = DATA_BUFFER_MAX_SIZE; 
    }

    memcpy(data_buffer[write_idx], data, size);
    data_lengths[write_idx] = size;

    write_idx = (write_idx + 1) % DATA_BUFFER_GROUP_COUNT;

    if (data_count < DATA_BUFFER_GROUP_COUNT) {
        data_count++;
    } else {
        read_idx = (read_idx + 1) % DATA_BUFFER_GROUP_COUNT;
    }
}

/**
 * @brief Initialization of the serial port.
 * 
 */
void app_sp_init(void)
{
    app_sp_spis_init();
    app_sp_uart_init();

    init = true;
}

/**
 * @brief Put data to the buffer of the serial port.
 * 
 * @param port The port to put.
 * @param data The data to put.
 * @param size The size of the data to put.
 */
void app_sp_put(uint8_t port, uint8_t const* data, uint16_t size)
{
    if (!init)
        return;

    if (port == SP_ALL)
    {
        app_sp_spis_put((uint8_t*)data, size);

        if (APP_SP_DEBUG_ENABLE)
            app_sp_uart_put((uint8_t*)data, size); //Send to debug port
    }
    else if (port == SP_SPI)
    {
        app_sp_spis_put((uint8_t*)data, size);
    }
    else if (port == SP_UART)
    {
        if (APP_SP_DEBUG_ENABLE)
            app_sp_uart_put((uint8_t*)data, size); //Send to debug port
    }
}

/**
 * @brief Process after receiving data from the buffer of the serial port.
 * 
 * @param port The port on which the data was received.
 * @param data Received data.
 * @param size The size of the received data.
 */
void app_sp_on_rx_data(uint8_t port, uint8_t const* data, uint16_t size)
{
    if (port == SP_UART && APP_SP_DEBUG_ENABLE == false)
    {
        return;
    }

    uint16_t data_size = size;
    response.resp_port = port;
    response.p_resp_cmd = app_cmd_parse_request((uint8_t*)data, data_size);
    if (response.p_resp_cmd == NULL)
    {
        if (*p_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            //app_ble_data_send((uint8_t*)data, (uint16_t*)&data_size);
            buffer_add((uint8_t*)data, data_size);
        }
    }
    else
    {
        //NRF_LOG_INFO("Response(%d), size(%d)", response.resp_port, response.p_resp_cmd->cmd_resp_len);
        //NRF_LOG_HEXDUMP_INFO((uint8_t*)response.p_resp_cmd->cmd_resp, response.p_resp_cmd->cmd_resp_len);
    }
}

/**
 * @brief The process after the serial port transaction is ready.
 * 
 * @param port The port that is ready for transmission.
 */
void app_sp_on_tx_ready(uint8_t port)
{
    if (response.p_resp_cmd != NULL && response.resp_port == port)
    {
        app_sp_put(response.resp_port, response.p_resp_cmd->cmd_resp, response.p_resp_cmd->cmd_resp_len);
        response.p_resp_cmd = NULL;
    }
}

/**
 * @brief Handler for transferring data from serial port to BLE.
 * 
 */
void app_sp_ble_handler(void)
{
    if (data_count == 0) {
        return;
    }

    uint8_t* data = data_buffer[read_idx];
    uint16_t size = data_lengths[read_idx];

    app_ble_data_send(data, &size);

    read_idx = (read_idx + 1) % DATA_BUFFER_GROUP_COUNT;
    data_count--;
}

