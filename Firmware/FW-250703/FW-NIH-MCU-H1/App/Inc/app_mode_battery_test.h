/**
 * @file app_mode_battery_test.h
 * @brief This file contains all the function prototypes for the app_mode_battery_test.c file
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_MODE_BATTERY_TEST_H_
#define INC_APP_MODE_BATTERY_TEST_H_
#include <stdint.h>

/**
 * @brief Measure, obtain and record battery voltages
 *
 * @param p_vbatA The battery voltage of battery 1, unit: mV
 * @param p_vbatB The battery voltage of battery 2, unit: mV
 */
void app_mode_battery_test_volt_get(uint16_t* p_vbatA, uint16_t* p_vbatB);

/**
 * @brief Handler for battery test mode
 *
 */
void app_mode_battery_test_handler(void);

#endif /* INC_APP_MODE_BATTERY_TEST_H_ */
