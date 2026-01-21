/**
 * @file app_sp_uart.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Applications of UART port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_sp_uart.h"
#include "custom_board.h"
#include "app_sp.h"

#include "app_uart.h"
#include "app_timer.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#define NRF_LOG_MODULE_NAME APP_SP_UART
#define NRF_LOG_LEVEL       3
#define NRF_LOG_INFO_COLOR  0
#define NRF_LOG_DEBUG_COLOR 0

#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include "nrf_log_ctrl.h"

#define UART_TX_BUF_SIZE                DATA_BUFFER_MAX_SIZE  /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                DATA_BUFFER_MAX_SIZE  /**< UART RX buffer size. */

#define UART_RX_DELAY_TIME_MS           10

APP_TIMER_DEF(rx_tmr);
static uint8_t rx_data[UART_RX_BUF_SIZE];
static uint16_t index = 0;

/**
 * @brief Timer handler for rx data.
 * 
 * @param p_context Unused
 */
static void uart_rx_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    app_sp_on_rx_data(SP_UART, (uint8_t*)rx_data, index);
    index = 0;
    app_sp_on_tx_ready(SP_UART);
}

/**
 * @brief Function for handling app_uart events.
 * 
 * @param p_event Event from the UART module
 */
static void uart_event_handle(app_uart_evt_t * p_event)
{
    uint32_t       err_code;
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&rx_data[index]));
            index++;
            APP_ERROR_CHECK(app_timer_start(rx_tmr, APP_TIMER_TICKS(UART_RX_DELAY_TIME_MS), NULL));
            break;

        case APP_UART_COMMUNICATION_ERROR:
            NRF_LOG_ERROR("APP_UART_COMMUNICATION_ERROR(%d), uart flash", p_event->data.error_communication);
            APP_ERROR_CHECK(app_uart_flush());
            break;

        case APP_UART_FIFO_ERROR:
            NRF_LOG_ERROR("APP_UART_FIFO_ERROR(%d), uart flash", p_event->data.error_code);
            APP_ERROR_CHECK(app_uart_flush());
            break;

        default:
            break;
    }
}

/**
 * @brief Function for initializing the UART module.
 * 
 */
void app_sp_uart_init(void)
{
    uint32_t err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = BLE_RX_PIN,
        .tx_pin_no    = BLE_TX_PIN,
        .rts_pin_no   = UART_PIN_DISCONNECTED,
        .cts_pin_no   = UART_PIN_DISCONNECTED,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
#if defined (UART_PRESENT)
        .baud_rate    = NRF_UART_BAUDRATE_115200
#else
        .baud_rate    = NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);

    APP_ERROR_CHECK(app_timer_create(&rx_tmr, APP_TIMER_MODE_SINGLE_SHOT, uart_rx_timer_handler));
}

/**
 * @brief Put data to the buffer of the UART.
 * 
 * @param data The data to put.
 * @param size The size of the data to put.
 */
void app_sp_uart_put(uint8_t* data, uint16_t size)
{
    uint32_t err_code;
    for (uint16_t i = 0; i < size; i++)
    {
        do
        {
            err_code = app_uart_put(data[i]);
            if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
                APP_ERROR_CHECK(err_code);
        } while (err_code == NRF_ERROR_BUSY);
    }
}