/**
 * @file main.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"

#include "app_ble.h"
#include "app_sp.h"
#include "app_cmd.h"

#if (APP_UNITY_TEST_ENABLE)
#include "test_app.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**
 * @brief Function for initializing the timer module.
 * 
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for initializing the nrf log module.
 * 
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**
 * @brief Function for initializing power management.
 * 
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for handling the idle state (main loop).
 * 
 */
static void idle_state_handle(void)
{
    if (PM_LESC_ENABLED)
    {
        APP_ERROR_CHECK(nrf_ble_lesc_request_handler());
    }

    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }

    app_ble_state_handler();
    app_sp_ble_handler();
}

/**
 * @brief Application main function.
 * 
 * @return int None
 */
int main(void)
{
    if ((NRF_UICR->APPROTECT & APPROTECT_DISABLE_DISABLE_Msk) == UICR_APPROTECT_PALL_HwDisabled)
    {
        NRF_APPROTECT->DISABLE = APPROTECT_DISABLE_DISABLE_SwDisable;
    }
    // Initialize.
    timers_init();
    power_management_init();
    log_init();
    NRF_LOG_INFO("BLE Initialize Start");
    NRF_LOG_FLUSH();

    app_ble_init();
    app_sp_init();

    NRF_LOG_INFO("BLE Initialize Finish");
    NRF_LOG_FLUSH();

#if (APP_UNITY_TEST_ENABLE)
    test_app();
#endif
    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}
