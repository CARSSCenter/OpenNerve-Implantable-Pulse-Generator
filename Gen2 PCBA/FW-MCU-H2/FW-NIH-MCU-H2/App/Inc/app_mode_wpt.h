/**
 * @file app_mode_wpt.h
 * @brief This file contains all the function prototypes for app_mode_wpt.c
 * @copyright Copyright (c) 2024
 */
#ifndef INC_APP_MODE_WPT_H_
#define INC_APP_MODE_WPT_H_
#include <stdint.h>

/* WPT charging thresholds */
#define WPT_BATT_ABSENT_THRESHOLD_MV     1000U    /*!< Below 1V = battery absent */
#define WPT_BATT_ABSENT_CONSECUTIVE      2U       /*!< 2 consecutive low samples to mark absent */
#define WPT_THERM_PAUSE_THRESHOLD_C      42.0f    /*!< °C — pause charging above this */
#define WPT_THERM_RESUME_THRESHOLD_C     42.0f    /*!< °C — resume when below this */
#define WPT_OVP_PAUSE_HOLD_MS            5000U    /*!< ms to hold in PAUSED after an OVP event before allowing resume */
#define WPT_THERM_SENSE_RESISTOR_OHM     49900.0f /*!< 49.9kΩ series sense resistor (104AP-2) */

/**
 * @brief Handler for WPT charging mode
 *
 */
void app_mode_wpt_handler(void);

/**
 * @brief Callback for the ms timer in WPT mode, called from HAL_IncTick
 *
 */
void app_mode_wpt_timer_cb(void);

#endif /* INC_APP_MODE_WPT_H_ */
