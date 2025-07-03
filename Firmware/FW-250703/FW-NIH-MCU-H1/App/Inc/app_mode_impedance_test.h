/**
 * @file app_mode_impedance_test.h
 * @brief This file contains all the function prototypes for the app_mode_impedance_test.c file
 * @copyright Copyright (c) 2024
 */
#ifndef APP_MODE_IMPEDANCE_TEST_H_
#define APP_MODE_IMPEDANCE_TEST_H_
#include <stdint.h>

/**
 * @brief Measure, obtain and record impedance
 *
 * @return _Float64 The impedance of the load, unit: ohm
 */
_Float64 app_mode_impedance_test_get(void);

/**
 * @brief Handler for impedance test mode
 * 
 */
void app_mode_impedance_test_handler(void);

#endif /* APP_MODE_IMPEDANCE_TEST_H_ */
