/**
 * @file test_app_ble.h
 * @author Louie Liu (louie@gimermed.com)
 * @brief Test the applications of Bluetooth
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef TEST_APP_BLE_H_
#define TEST_APP_BLE_H_

#include "unity.h"

/**
 * @brief Run all tests in Bluetooth idle state
 * 
 */
void test_run_app_ble_on_idle (void);

/**
 * @brief Run all the test items when the Bluetooth is connected
 * 
 */
void test_run_app_ble_on_connected (void);

/**
 * @brief Run all the test items when the Bluetooth is disconnected
 * 
 */
void test_run_app_ble_on_disconnected (void);

#endif