/**
 * @file test_app_sp.c
 * @author Louie Liu (louie@gimermed.com)
 * @brief Test the applications of serial port
 * @version 0.1.00
 * @date 2023-06-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "test_app_sp.h"

#include "app_sp.h"
#include <stdbool.h>
#include "nrf_delay.h"
#include "custom_board.h"

/**
 * @brief Test the input data application of SPIS
 * 
 */
static void test_app_sp_spis_put (void)
{
    uint8_t data[254];
    uint16_t length = sizeof(data);
    for(int i=0;i<length;i++)
        data[i] = i+1;
    
    bool req = false;
    TEST_ASSERT_TRUE(nrf_gpio_pin_out_read(BLE_RDY_PIN));
    app_sp_put(SP_SPI, data, length);
    for(uint32_t i=0;i<10000;i++)
    {
        req |= nrf_gpio_pin_out_read(BLE_REQ_PIN);
    }

    TEST_ASSERT_TRUE(req);
    TEST_ASSERT_TRUE(nrf_gpio_pin_out_read(BLE_RDY_PIN));
    nrf_delay_ms(500);
}

/**
 * @brief Run all test items for serial port applications
 * 
 */
void test_run_app_sp (void)
{
    UnityPrint("***********************");
    UNITY_PRINT_EOL();

    UnityBegin("[SP]");
    RUN_TEST(test_app_sp_spis_put, __LINE__);
    UNITY_END();
    nrf_delay_ms(500);
}