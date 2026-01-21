/**
 * @file test_app.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Test all applications
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "test_app.h"

#include "test_app_ble.h"
#include "test_app_sp.h"
#include "test_app_cmd.h"

#include "nrf_delay.h"

/**
 * @brief These functions are intended to be called before each test.
 * 
 */
void setUp (void)
{

}

/**
 * @brief These functions are intended to be called after each test.
 * 
 */
void tearDown (void)
{
    nrf_delay_ms(500);
}

/**
 * @brief Test all units of the application
 * 
 */
void test_app(void)
{
    UnityPrint("Unity Test Start...");

    test_run_app_sp();
    test_run_app_cmd();
    test_run_app_ble_on_idle();
    test_run_app_ble_on_connected();
    test_run_app_ble_on_disconnected();

    UnityPrint("...Unity Test Finish");
    UnityPrint("Unity Test Finish...");
}
